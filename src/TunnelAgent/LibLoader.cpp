/*
 * loadtest.cpp
 *
 *  Created on: 2017年11月9日
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
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include "strategy_shared_comm.h"
#include "utils.h"
using namespace std;


static bool g_is_delete_after_load = false;

int load(string name, string file_path, string conf)
{
	dlerror();
	const char * perr = NULL;
	void *pdl_handle = dlopen(file_path.c_str(), RTLD_NOW);

	if(g_is_delete_after_load) unlink(file_path.c_str());		// remove it immediately

	if(pdl_handle == NULL)
	{
		perr = dlerror();
		printf("load %s err: %s\n", file_path.c_str(), perr);
		exit(-1);
	}

	dlerror();
	typedef void (*setnamefunc)(const char*);
	setnamefunc setf = (setnamefunc)dlsym(pdl_handle, "setStrategyName");
	if((perr = dlerror()) != NULL)
	{
		printf("so %s err: %s\n", file_path.c_str(), perr);
		dlclose(pdl_handle);
		exit(-1);
	}
	setf(name.c_str());

	dlerror();
	typedef bool (*loadconffunc)(const char*);
	loadconffunc loadconff = (loadconffunc)dlsym(pdl_handle, "loadConf");
	if((perr = dlerror()) != NULL)
	{
		printf("so %s err: %s\n", file_path.c_str(), perr);
		dlclose(pdl_handle);
		exit(-1);
	}
	if(!loadconff(conf.c_str()))
	{
		printf("so %s load config err\n", file_path.c_str());
		dlclose(pdl_handle);
		exit(-1);
	}

	dlerror();
	typedef int (*runfunc)(void);
	runfunc func = (runfunc)dlsym(pdl_handle, "run");
	if((perr = dlerror()) != NULL)
	{
		printf("so %s err: %s\n", file_path.c_str(), perr);
		dlclose(pdl_handle);
		exit(-1);
	}

	printf("%s ready to run.\n", file_path.c_str());

	return func();
}

void Usage(const char *cmd)
{
	printf("Usage: %s -n name -f lib -c config [-d]\n", cmd);
	exit(-1);
}

int main(int argc, char *argv[])
{
	int opt = 0;
	string cfg;
	string lib;
	string name;

	while ((opt = getopt(argc, argv, "f:c:n:d")) != -1)
	{
		switch(opt)
		{
		case 'n':
			name = optarg;
			break;
		case 'f':
			lib = optarg;
			break;
		case 'c':
			cfg = optarg;
			break;
		case 'd':
			g_is_delete_after_load = true;
			break;
		default: ;
		}
	}

	if(lib.empty() || cfg.empty() || name.empty())
	{
		Usage(argv[0]);
	}

	ifstream in(cfg.c_str());
	if(!in)
	{
		cerr << "config file " << cfg << " not exists." << endl;
		return -1;
	}

	string content((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
	in.close();

	if(g_is_delete_after_load) unlink(cfg.c_str());  // remove config

	setProcTitle(name, argv);
	return load(name, lib, content);
}


