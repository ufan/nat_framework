#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>

#include "CClient.h"
using namespace std;


void Usage(const char *cmd)
{
	printf("Usage: %s -f config [-c command -c ...]\n"
			"\t-f configure file path\n"
			"\t-c command\n", cmd);

	exit(-1);
}


int main(int argc, char *argv[])
{
	int opt = 0;
	string cfg;
	vector<string> extra_cmd;

	while ((opt = getopt(argc, argv, "f:c:")) != -1)
	{
		switch(opt)
		{
		case 'f':
			cfg = optarg;
			break;

		case 'c':
			extra_cmd.push_back(optarg);
			break;

		default:
			Usage(argv[0]);
		}
	}

	if(cfg.empty())
	{
		Usage(argv[0]);
	}

	CClient client;
	if(client.init(cfg))
	{
		client.run(extra_cmd);
	}

	return 0;
}

