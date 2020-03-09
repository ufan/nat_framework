/*
 * CPythonExecutor.cpp
 *
 *  Created on: 2017年9月28日
 *      Author: hongxu
 */

#include "CPythonExecutor.h"
#include <pthread.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdlib.h>
#include <python2.7/Python.h>
#include "CStrategyManager.h"
#include "CTunnelAgent.h"
#include "CWaiter.h"
#include "protocol.h"
#include "Logger.h"
#include "ev.h"
#include "utils.h"
#include "CConfig.h"
#include "json.hpp"
using json = nlohmann::json;


CPythonExecutor::CPythonExecutor(CWaiter *p_owner) : CFileReciever(p_owner), state_(STATE_RECV_SCRIPT)
{

}

CPythonExecutor::~CPythonExecutor()
{

}

int CPythonExecutor::run(string &pkg)
{
	int ret = STATE_ABORT;
	switch(state_)
	{
	case STATE_RECV_SCRIPT:
		ret = CFileReciever::run(pkg);
		if(ret == STATE_FINISH)
		{
			state_ = STATE_RECV_CONF;
			p_conf_reciever_.reset(new CFileReciever(p_owner_));
			return STATE_PENDING;
		}
		else return ret;
		break;
	case STATE_RECV_CONF:
		ret = p_conf_reciever_->run(pkg);
		if(ret == STATE_FINISH)
		{
			state_ = STATE_RECV_WORKDIR;
			conf_ = p_conf_reciever_->getContent();
			p_dir_reciever_.reset(new CDirReciever(p_owner_));
			return STATE_PENDING;
		}
		else return ret;
		break;
	case STATE_RECV_WORKDIR:
		ret = p_dir_reciever_->run(pkg);
		if(ret == STATE_FINISH)
		{
			state_ = STATE_RECV_ARGS;
			return STATE_PENDING;
		}
		else return ret;
		break;
	case STATE_RECV_ARGS:
		state_ = STATE_RUN;
		args_ = pkg;
		/* no break */
	case STATE_RUN:
	{
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
			LOG_INFO("IGNORED: %s is already running。", name_.c_str());

			string info("the same name process is already running.\t\t\t" SWARN("[IGNORED]" "\n"));
			sendToClient(info);
			string dirpath = p_dir_reciever_->getDirPath();
			if(dirpath.size()) loopDelDir(dirpath);
			ret = STATE_FINISH;
		}
		else ret = execute();
		break;
	}
	}

	if(ret == STATE_FINISH)			p_owner_->sayFinish();
	else if(ret == STATE_ABORT) 	p_owner_->sayFinish(TYPE_FINISH_FAIL);

	return ret;
}

int CPythonExecutor::execute()
{
	CConfig *conf = CTunnelAgent::instance()->getConfig();
	string dirpath = p_dir_reciever_->getDirPath();
	string base_dir = conf->getVal<string>("COMMON", "base_dir");
	if(base_dir[base_dir.size() - 1] != '/')
	{
		base_dir.push_back('/');
	}

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
		return STATE_FINISH;
	}
	else	 // child
	{
		string python_bin = conf->getVal<string>("PYTHON", "bin");
		string pyloader = conf->getVal<string>("COMMON", "pyloader");
		string lib_dir = conf->getVal<string>("COMMON", "lib_dir", base_dir + "lib");
		string ld_env("LD_LIBRARY_PATH=");
		ld_env += lib_dir;

		string cfg_env("STG_CFG=");
		cfg_env += conf_;

		string content_env("STG_CONTENT=");
		content_env += content_;

		string del_gap = conf->getVal<string>("PYTHON", "clean_interval", "5");

		const char* p_env[] = {ld_env.c_str(), cfg_env.c_str(), content_env.c_str(), NULL};
		const char* p_args[] = {pyloader.c_str(), "-n", name_.c_str(), "-p", python_bin.c_str(), "-s", del_gap.c_str(), NULL, NULL, NULL};
		if(dirpath.size())
		{
			p_args[7] = "-w";
			char *real_path = realpath(dirpath.c_str(), NULL);
			dirpath = real_path;
			free(real_path);
			p_args[8] = dirpath.c_str();
		}

		for(int i = 0; i < 1024; i++)
		{
			close(i);
		}
		string loader_log = conf->getVal<string>("COMMON", "loader_log_dir");
		loader_log += "/PyLoader.log";
		int fd = open(loader_log.c_str(), O_CREAT|O_RDWR|O_APPEND, 0664);
		if(fd >= 0)
		{
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		chdir(base_dir.c_str());
		chdir(lib_dir.c_str());

		if(0 > execvpe(pyloader.c_str(), (char* const*)p_args, (char* const*)p_env))
		{
			LOG_ERR("load py %s err: %s", name_.c_str(), strerror(errno));
			if(dirpath.size()) loopDelDir(dirpath);
			exit(-1);
		}
	}
	return STATE_ABORT;
}

void CPythonExecutor::child_cb(EV_P_ ev_child* w, int revents)
{
	CStrategyManager::exitStg(((tChildWatcher*)w)->name);

	ev_child_stop (EV_DEFAULT_ w);
	LOG_INFO("py %s exits, status %d.", ((tChildWatcher*)w)->name.c_str(),
			WEXITSTATUS(w->rstatus));
	delete ((tChildWatcher*)w);
}

