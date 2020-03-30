/*
 * CLibLoader.cpp
 *
 *  Created on: 2017年10月4日
 *      Author: hongxu
 */

#include "CExecLoader.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include "CStrategyManager.h"
#include "CTunnelAgent.h"
#include "CWaiter.h"
#include "CConfig.h"
#include "utils.h"
#include "json.hpp"
using json = nlohmann::json;

inline void delDir(string dir)
{
	if(dir.size()) loopDelDir(dir);
}

CExecLoader::CExecLoader(CWaiter *p_owner) : CFileReciever(p_owner), exec_state_(ST_RECV_FILE)
{

}

CExecLoader::~CExecLoader()
{

}

int CExecLoader::run(string &pkg)
{
	int ret = STATE_ABORT;
	switch(exec_state_)
	{
	case ST_RECV_FILE:
		ret = CFileReciever::run(pkg);
		if(ret == STATE_FINISH)
		{
			exec_state_ = ST_RECV_CONFIG;
			p_conf_reciever_.reset(new CFileReciever(p_owner_));
			p_owner_->sayFinish(TYPE_FINISH_ACK);
			return STATE_PENDING;
		}
		else return ret;
		break;
	case ST_RECV_CONFIG:
		ret = p_conf_reciever_->run(pkg);
		if(ret == STATE_FINISH)
		{
			exec_state_ = ST_RECV_DIR;
			conf_ = p_conf_reciever_->getContent();
			p_conf_reciever_.reset();

			// check config and process here
			try
			{
				json j = json::parse(conf_);
				name_ = j["name"];
			}catch(...)
			{
				LOG_INFO("strategy config parse err.");
				string info(SERR("strategy config parse err." "\n"));
				sendToClient(info);
				ret = STATE_ABORT;
				break;
			}
			if(findProcessByCmdLine(name_) >= 0)
			{
				LOG_INFO("IGNORED: %s is already running.", name_.c_str());
				string info("the same name process is already running.\t\t\t" SWARN("[IGNORED]" "\n"));
				sendToClient(info);
				ret = STATE_FINISH;
				break;
			}
			p_dir_reciever_.reset(new CDirReciever(p_owner_));
			p_owner_->sayFinish(TYPE_FINISH_ACK);
			return STATE_PENDING;
		}
		else return ret;
		break;
	case ST_RECV_DIR:
		ret = p_dir_reciever_->run(pkg);
		if(ret == STATE_FINISH)
		{
			exec_state_ = ST_RECV_CMD;
			workdir_ = p_dir_reciever_->getDirPath();
			if(workdir_.size())
			{
				char *real_path = realpath(workdir_.c_str(), NULL);
				if(real_path) {workdir_ = real_path; free(real_path);}
			}
			p_dir_reciever_.reset();
			p_owner_->sayFinish(TYPE_FINISH_ACK);
			return STATE_PENDING;
		}
		else return ret;
		break;
	case ST_RECV_CMD:
	{
		args_ = pkg;
		ret = load();
		if(STATE_ABORT == ret)
		{
			delDir(workdir_);
		}
		break;
	}
	}

	if(ret == STATE_FINISH)			p_owner_->sayFinish();
	else if(ret == STATE_ABORT) 	p_owner_->sayFinish(TYPE_FINISH_FAIL);
	return ret;
}

int CExecLoader::load()
{
	CConfig *conf = CTunnelAgent::instance()->getConfig();
	string base_dir = conf->getVal<string>("COMMON", "base_dir");
	if(base_dir[base_dir.size() - 1] != '/')
	{
		base_dir.push_back('/');
	}
	string dir_path = base_dir + ".tmp";			// for save .so lib
	string file_path = dir_path + '/' + name_;
	char *real_path = realpath(file_path.c_str(), NULL);
	if(real_path) {file_path = real_path; free(real_path);}

	if(!checkTmpDir(dir_path)) return STATE_ABORT;

	if(!CStrategyManager::registerStg(name_))
	{
		LOG_ERR("register %s err", name_.c_str());
		string info("register ");
		info += name_ + " err.\n";
		sendToClient(info);
		return STATE_ABORT;
	}

	pid_t pid = fork();
	if(pid < 0)
	{
		LOG_ERR("fork err: %s", strerror(errno));
		return STATE_ABORT;
	}
	if(pid > 0)
	{
		tChildWatcher *p_child_watcher = new tChildWatcher;		// this object will reserve until callback delete it.
		p_child_watcher->name = name_;
		ev_child_init ((ev_child*)p_child_watcher, child_cb, pid, 0);
		ev_child_start (EV_DEFAULT_ (ev_child*)p_child_watcher);

		double del_sec = conf->getVal<double>("COMMON", "clean_interval");
		tDelWatcher *p_del_watcher = new tDelWatcher;
		p_del_watcher->file = file_path;
		p_del_watcher->workdir = workdir_;
		ev_timer_init (&(p_del_watcher->timer), timeout_cb, del_sec, 0.);
		ev_timer_start(EV_DEFAULT, &(p_del_watcher->timer));

		return STATE_FINISH;
	}
	else	 // child
	{
		runChild(file_path);
	}

	return STATE_ABORT;
}

bool CExecLoader::checkTmpDir(string dir_path)
{
	struct stat stbuf;
	if(stat(dir_path.c_str(), &stbuf) < 0)
	{
		if(errno != ENOENT)
		{
			LOG_ERR("stat %s err: %s", dir_path.c_str(), strerror(errno));
			return false;
		}
		else if(mkdir(dir_path.c_str(), 0600) < 0)
		{
			LOG_ERR("mkdir %s err: %s", dir_path.c_str(), strerror(errno));
			return false;
		}
	}
	else if(!S_ISDIR(stbuf.st_mode))
	{
		LOG_ERR("%s is not a directory.", dir_path.c_str());
		return false;
	}

	return true;
}

void CExecLoader::runChild(string file_path)
{
	// store exe
	int fd = open(file_path.c_str(), O_CREAT|O_TRUNC|O_CLOEXEC|O_RDWR, 0600);
	if(fd < 0)
	{
		delDir(workdir_);
		LOG_ERR("create file %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
	if(write(fd, content_.data(), content_.size()) != content_.size())
	{
		close(fd);
		unlink(file_path.c_str());
		delDir(workdir_);
		LOG_ERR("write file %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
	close(fd);

	CConfig *conf = CTunnelAgent::instance()->getConfig();
	string base_dir = conf->getVal<string>("COMMON", "base_dir");

	string lib_env = string("LD_LIBRARY_PATH=") + conf->getVal<string>("COMMON", "lib_dir", base_dir + "/lib");
	string pythonpath = string("PYTHONPATH=") + conf->getVal<string>("COMMON", "python_path", base_dir + "/lib");
	if(workdir_.size())
	{
		lib_env += string(":") + workdir_;
		pythonpath += string(":") + workdir_;
	}
	string cfg_env = string("STG_CFG=") + conf_;
	const char* p_env[] = {lib_env.c_str(), pythonpath.c_str(), cfg_env.c_str(), NULL};

	vector<string> cmd_split;
	splitCmdLine(args_, cmd_split);
	const char** p_args = new const char*[cmd_split.size() + 2];
	p_args[0] = file_path.c_str();
	for(uint32_t i = 0; i < cmd_split.size(); ++i) p_args[i+1] = cmd_split[i].c_str();
	p_args[cmd_split.size() + 1] = nullptr;

	for(int i = 0; i < 1024; i++)
	{
		close(i);
	}
	string loader_log = conf->getVal<string>("COMMON", "exe_log_dir");
	if(loader_log.back() != '/') loader_log.push_back('/');
	loader_log += name_ + "_stdio.log";
	fd = open(loader_log.c_str(), O_CREAT|O_RDWR|O_APPEND, 0664);
	if(fd >= 0)
	{
		dup2(fd, 1);
		dup2(fd, 2);
		close(fd);
	}

	if(workdir_.size()) chdir(workdir_.c_str());
	if(0 > execvpe(file_path.c_str(), (char* const*)p_args, (char* const*)p_env))
	{
		unlink(file_path.c_str());
		delDir(workdir_);
		LOG_ERR("execute %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
}

void CExecLoader::child_cb(EV_P_ ev_child* w, int revents)
{
	CStrategyManager::exitStg(((tChildWatcher*)w)->name);

	ev_child_stop (EV_DEFAULT_ w);
	LOG_INFO("process %s exits, status %d.", ((tChildWatcher*)w)->name.c_str(),
			WEXITSTATUS(w->rstatus));
	delete ((tChildWatcher*)w);
}

void CExecLoader::timeout_cb(EV_P_ ev_timer *w, int revents)
{
	ev_timer_stop(EV_DEFAULT, w);
	unlink(((tDelWatcher*)w)->file.c_str());
	delDir(((tDelWatcher*)w)->workdir);
	delete ((tDelWatcher*)w);
}

