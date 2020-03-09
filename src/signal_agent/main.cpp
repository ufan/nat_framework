/*
 * main.cpp
 *
 *  Created on: Jun 27, 2018
 *      Author: hongxu
 */

#include <fstream>
#include "CSignalAgent.h"
#include "utils.h"
using namespace std;


static void ev_terminate_cb(EV_P_ ev_signal *w, int events)
{
	ev_break (EV_DEFAULT, EVBREAK_ALL);
	LOG_INFO("got terminate signal. process exits.");
	exit(0);
}

static void setTerminateCB()
{
    // start ev
    struct ev_loop *loop = ev_default_loop (EVBACKEND_EPOLL | EVFLAG_NOENV);

    static ev_signal exitsig;
    ev_signal_init (&exitsig, ev_terminate_cb, SIGTERM);
    ev_signal_start (loop, &exitsig);
    ev_unref (loop);

    static ev_signal intsig;
    ev_signal_init (&intsig, ev_terminate_cb, SIGINT);
    ev_signal_start (loop, &intsig);
    ev_unref (loop);
}

void usage(const char *cmd)
{
	printf("Usage: %s -f config [-n]\n"
			"\t-f configure file path\n"
			"\t-n do not run as daemon process\n", cmd);

	exit(-1);
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		usage(argv[0]);
		return -1;
	}

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
		usage(argv[0]);
	}

	if(daemon)
	{
		setDaemon();
	}

	ifstream in(cfg);
	if(!in)
	{
		fprintf(stderr, "read config file %s err.\n", cfg.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	json j_conf = json::parse(content);

	initLogger(j_conf["log4cplus"]);
	setTerminateCB();

	CSignalAgent agent;
	ASSERT_RET(agent.init(j_conf), false);
	agent.run();

    LOG_INFO("start.");
    ev_run (EV_DEFAULT, 0);		// loop forever, so the above stack variables is safe to use until exist.
    LOG_INFO("END RUNNING.");
}


