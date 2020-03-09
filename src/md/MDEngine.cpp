/*
 * MDEngine.cpp
 *
 *  Created on: 2018年5月9日
 *      Author: hongxu
 */

#include <iostream>
#include <fstream>
#include <csignal>
#include "CMDEngineCtp.h"
#include "CMDEngineEES.h"
#include "CMDEngineEESOnload.h"
#include "utils.h"
#include "Logger.h"

IMDEngine *p_engine = nullptr;

void signal_handler(int signum)
{
	if(p_engine) p_engine->stop();
}

void setup_signal_callback()
{
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGHUP, signal_handler);
    std::signal(SIGQUIT, signal_handler);
    std::signal(SIGKILL, signal_handler);
}

IMDEngine* createEngine(string type)
{
	if(type == "ctp")
	{
		return new CMDEngineCtp();
	}
	else if(type == "ees")
	{
		return new CMDEngineEES();
	}
	else if(type == "onload_ees")
	{
		return &CMDEngineEESOnload::instance_;
	}

	ALERT("unknown engine type: %s", type.c_str());
	return nullptr;
}

bool run(string conf)
{
	ifstream in(conf.c_str());
	if(!in)
	{
		ALERT("config file %s not exists.", conf.c_str());
		return false;
	}
	json j_conf;
	in >> j_conf;
	in.close();

	initLogger(j_conf["/MDEngine/log4cplus"_json_pointer]);

	string type = j_conf["/MDEngine/type"_json_pointer];
	p_engine = createEngine(type);
	if(p_engine == nullptr) return false;

	if(!p_engine->initEngine(j_conf))
	{
		ALERT("init engine failed");
		return false;
	}

	setup_signal_callback();
	p_engine->runEngine();
	return true;
}

void Usage(const char *cmd)
{
	printf("Usage: %s -f config [-n]\n"
			"\t-f configure file path\n"
			"\t-n do not run as daemon process\n", cmd);

	exit(-1);
}

int main(int argc, char *argv[])
{
	bool daemon = true;
	string cfg;

	int opt = 0;
	while ((opt = getopt(argc, argv, "nf:")) != -1)
	{
		switch(opt)
		{
		case 'n':
			daemon = false;
			break;
		case 'f':
			cfg = optarg;
			break;
		default: ;
		}
	}

	if(cfg.empty())
	{
		Usage(argv[0]);
	}

	if(daemon)
	{
		setDaemon();
	}

	if(!run(cfg)) return -1;
	return 0;
}


