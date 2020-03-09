/*
 * loadtest.cpp
 *
 *  Created on: 2017年11月9日
 *      Author: hongxu
 */
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <python2.7/Python.h>
#include "utils.h"
using namespace std;

struct tDelDir
{
	int  		interval;
	string		dir;
};

static void* delThread(void *arg)
{
	pthread_detach(pthread_self());

	tDelDir *p = (tDelDir*)arg;
	if(p->interval > 0)
	{
		sleep(p->interval);
		loopDelDir(p->dir);
	}
	delete p;
	return NULL;
}

static bool delWorkdir(int interval, string dir, pthread_t &tid)
{
	tDelDir *p_del = new tDelDir;
	p_del->interval = interval;
	p_del->dir = dir;
	if(0 != pthread_create(&tid, NULL, delThread, p_del))
	{
		perror("create clean workdir thread err:");
		return false;
	}
	return true;
}

static int load(string name, string content, string conf, string python, string workdir)
{
	Py_SetProgramName((char*)python.c_str());
	Py_Initialize();

	string arg0("./");
	arg0 += name;
	char *p_argv = (char*)arg0.c_str();
	PySys_SetArgv(1, &p_argv);

	string tmpstr("import pystrategy\npystrategy.setStrategyName('");
	tmpstr += name + "')\n";
	PyRun_SimpleString(tmpstr.c_str());

	tmpstr = "pystrategy.loadConf(r'''";
	tmpstr += conf + "''')\n";
	PyRun_SimpleString(tmpstr.c_str());

	if(workdir.size())
	{
		chdir(workdir.c_str());
		tmpstr = "import sys\nsys.path.append('";
		tmpstr += workdir + "')\n";
		PyRun_SimpleString(tmpstr.c_str());
	}

	int ret = PyRun_SimpleString(content.c_str());
	Py_Finalize();

	exit(ret);
}

static void Usage(const char *cmd)
{
	printf("Usage: %s -n name -p python_bin [-w workdir]\n", cmd);
	exit(-1);
}

int main(int argc, char *argv[])
{
	int opt = 0;
	string cfg;
	string content;
	string name;
	string python_bin;
	string workdir;
	int del_interval = -1;

	while ((opt = getopt(argc, argv, "n:p:w:s:")) != -1)
	{
		switch(opt)
		{
		case 'n':
			name = optarg;
			break;
		case 'p':
			python_bin = optarg;
			break;
		case 'w':
			workdir = optarg;
			break;
		case 's':
			del_interval = atoi(optarg);
			break;
		default: ;
		}
	}

	char *p_cfg = getenv("STG_CFG");
	char *p_content = getenv("STG_CONTENT");
	if(!p_cfg || !p_content)
	{
		printf("please set STG_CFG and STG_CONTENT environment variable.\n");
		return -1;
	}

	cfg = p_cfg;
	content = p_content;

	if(content.empty() || cfg.empty() || name.empty() || python_bin.empty())
	{
		Usage(argv[0]);
	}

	setProcTitle(name, argv);

	if(workdir.size())
	{
		char *real_path = realpath(workdir.c_str(), NULL);
		if(real_path == NULL)
		{
			fprintf(stderr, "get %s realpath err: %s", workdir.c_str(), strerror(errno));
			return -1;
		}
		workdir = real_path;
		free(real_path);
	}

	bool is_create_thread = false;
	pthread_t tid;
	if(del_interval > 0 && workdir.size())
	{
		is_create_thread = delWorkdir(del_interval, workdir, tid);
	}

	int ret = load(name, content, cfg, python_bin, workdir);

	if(is_create_thread)
	{
		pthread_join(tid, NULL);
	}

	return ret;
}


