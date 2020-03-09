/*
 * CLibLoader.cpp
 *
 *  Created on: 2017年10月4日
 *      Author: hongxu
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include "CLibLoader.h"
#include "CTunnelAgent.h"
#include "CConfig.h"
#include "utils.h"
#include "CWaiter.h"
#include "CStrategyManager.h"


CLibLoader::CLibLoader(CWaiter *p_owner) : CFileReciever(p_owner), p_conf_reciever_(NULL)
{

}

CLibLoader::~CLibLoader()
{
	if(p_conf_reciever_)
	{
		delete p_conf_reciever_;
	}
}

int CLibLoader::run(string &pkg)
{
	int ret = STATE_ABORT;

	if(p_conf_reciever_)
	{
		ret = p_conf_reciever_->run(pkg);
	}
	else
	{
		ret = CFileReciever::run(pkg);
	}

	if(ret == STATE_FINISH)
	{
		if(p_conf_reciever_ == NULL)
		{
			p_conf_reciever_ = new CFileReciever(p_owner_);
			return STATE_PENDING;
		}
		else
		{
			conf_ = p_conf_reciever_->getContent();
			delete p_conf_reciever_;
			p_conf_reciever_ = NULL;

			if(findProcessByCmdLine(name_) >= 0)
			{
				LOG_INFO("IGNORED: %s is already running。", name_.c_str());

				string info("the same name process is already running。\t\t\t" SWARN("[IGNORED]" "\n"));
				sendToClient(info);
			}
			else ret = load();
		}
	}

	if(ret == STATE_FINISH)			p_owner_->sayFinish();
	else if(ret == STATE_ABORT) 	p_owner_->sayFinish(TYPE_FINISH_FAIL);

	return ret;
}

int CLibLoader::load()
{
	CConfig *conf = CTunnelAgent::instance()->getConfig();
	string base_dir = conf->getVal<string>("COMMON", "base_dir");
	if(base_dir[base_dir.size() - 1] != '/')
	{
		base_dir.push_back('/');
	}
	string dir_path = base_dir + ".tmp";			// for save .so lib

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
		return STATE_FINISH;
	}
	else	 // child
	{
		runChild(dir_path);
	}

	return STATE_ABORT;
}

bool CLibLoader::checkTmpDir(string dir_path)
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

void CLibLoader::runChild(string dir_path)
{
	CConfig stg_conf;
	if(!stg_conf.initConfStr(conf_))
	{
		LOG_ERR("load strategy %s config err.", name_.c_str());
		exit(-1);
	}

	// store config
	string conf_path = dir_path + '/' + name_ + ".cnf";
	int fd = open(conf_path.c_str(), O_CREAT|O_TRUNC|O_CLOEXEC|O_RDWR, 0600);
	if(fd < 0)
	{
		LOG_ERR("create file %s err: %s", conf_path.c_str(), strerror(errno));
		exit(-1);
	}
	if(write(fd, conf_.data(), conf_.size()) != conf_.size())
	{
		close(fd);
		unlink(conf_path.c_str());
		LOG_ERR("write file %s err: %s", conf_path.c_str(), strerror(errno));
		exit(-1);
	}
	close(fd);

	// store so
	string file_path = dir_path + '/' + name_;
	fd = open(file_path.c_str(), O_CREAT|O_TRUNC|O_CLOEXEC|O_RDWR, 0600);
	if(fd < 0)
	{
		unlink(conf_path.c_str());
		LOG_ERR("create file %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
	if(write(fd, content_.data(), content_.size()) != content_.size())
	{
		close(fd);
		unlink(file_path.c_str());
		unlink(conf_path.c_str());
		LOG_ERR("write file %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
	close(fd);

	CConfig *conf = CTunnelAgent::instance()->getConfig();
	string libloader = conf->getVal<string>("COMMON", "libloader");

	string base_dir = conf->getVal<string>("COMMON", "base_dir");
	string lib_dir = stg_conf.getVal<string>("COMMON", "lib_dir", base_dir + "/lib");

	string lib_env("LD_LIBRARY_PATH=");
	lib_env += lib_dir;
	const char *p_env[] = {lib_env.c_str(), NULL};

	for(int i = 0; i < 1024; i++)
	{
		close(i);
	}
	string loader_log = conf->getVal<string>("COMMON", "loader_log_dir");
	loader_log += "/LibLoader.log";
	fd = open(loader_log.c_str(), O_CREAT|O_RDWR|O_APPEND, 0664);
	if(fd >= 0)
	{
		dup2(fd, 1);
		dup2(fd, 2);
		close(fd);
	}

	chdir(base_dir.c_str());
	chdir(lib_dir.c_str());

	if(0 > execle(libloader.c_str(), libloader.c_str(), "-n", name_.c_str(), "-f", file_path.c_str(), "-c", conf_path.c_str(), "-d", NULL, p_env))
	{
		unlink(file_path.c_str());
		unlink(conf_path.c_str());
		LOG_ERR("load lib %s err: %s", file_path.c_str(), strerror(errno));
		exit(-1);
	}
}

void CLibLoader::child_cb(EV_P_ ev_child* w, int revents)
{
	CStrategyManager::exitStg(((tChildWatcher*)w)->name);

	ev_child_stop (EV_DEFAULT_ w);
	LOG_INFO("so %s exits, status %d.", ((tChildWatcher*)w)->name.c_str(),
			WEXITSTATUS(w->rstatus));
	delete ((tChildWatcher*)w);
}

