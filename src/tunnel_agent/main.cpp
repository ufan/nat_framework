#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include "CTunnelAgent.h"
#include "utils.h"
using namespace std;

char **g_os_argv = NULL;

void Usage(const char *cmd)
{
	printf("Usage: %s -f config [-n]\n"
			"\t-f configure file path\n"
			"\t-n do not run as daemon process\n", cmd);

	exit(-1);
}


int main(int argc, char **argv)
{
	g_os_argv = argv;

	bool daemon = true;
	int opt = 0;
	string cfg;

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

	if(CTunnelAgent::instance()->init(cfg))
	{
		CTunnelAgent::instance()->run();
	}
	else
	{
		fprintf(stderr, "server start failed!\n");
		return -1;
	}

	return 0;
}

