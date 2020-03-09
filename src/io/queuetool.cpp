/*
 * test.cpp
 *
 *  Created on: Sep 6, 2017
 *      Author: hongxu
 */

#include <iostream>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "CMemoryQueueBase.h"
#include "CMemQueue.h"
using namespace IPC;
using namespace std;

#define OBJ_CNT 1024

struct stObj
{
	int i;
	int64_t d;
	char c;
};

stObj g_objs[OBJ_CNT];
CMemQueue g_queue;


void testWrite()
{
	for(int i = 0;;i++)
	{
		g_queue.write(g_objs + (i % OBJ_CNT));
		if(i % 23 == 0)
		{
			usleep(30000);
		}
	}
}


void testRead()
{
	while(1)
	{
		stObj stq;
		if(g_queue.read(&stq))
		{
			printf("%d %lld %c\n", stq.i, (long long)stq.d, stq.c);
		}
		else
		{
			usleep(20000);		// 10ms
		}
	}
}

bool init()
{
	for(int i = 0; i < OBJ_CNT; i++)
	{
		g_objs[i].i = i;
		g_objs[i].d = i + 1;
		g_objs[i].c = 'a' + (i % 26);
	}
	return CMemoryQueueBase::instance()->init(IO_QUEUE_BASE_FILE);
}

void getSize()
{
	cout << "base structure size: " << sizeof(CMemoryQueueBase::stQueueBase) << endl;
}

void list()
{
	vector<string> v;
	CMemoryQueueBase::instance()->list(v);
	for(vector<string>::iterator itr = v.begin(); itr != v.end(); ++itr)
	{
		uint32_t key = CMemoryQueueBase::instance()->getShmKeyByName(*itr);
		printf("key:0x%08x name:%s ", key, itr->c_str());

		CMemQueue q;
		if(q.init(*itr, 0, 0))
		{
			q.printInfo();
		}
		else
		{
			cout << "err: can't init queue " << *itr << endl;
		}
	}
}

void read()
{
	static char buf[4096 * 2];

	while(1)
	{
		int iret = 0;
		if((iret = g_queue.read(buf)) > 0)
		{
			for(int i = 0; i < iret; i++)
			{
				if(isprint(buf[i]))
				{
					putchar(buf[i]);
				}
				else
				{
					printf("\\%02x", (unsigned char)buf[i]);
				}
			}
			printf("\n--------\n");
		}
		else
		{
			usleep(10000);		// 10ms
		}
	}
}

void clean(const char *name = NULL)
{
	if(name == NULL)
	{
		printf("Warning: sure to clean all queues?[Y|n]");
		char c = 'n';
		scanf("%c", &c);
		if(c != 'Y')
		{
			exit(-1);
		}
	}

	vector<string> v;
	CMemoryQueueBase::instance()->list(v);
	for(vector<string>::iterator itr = v.begin(); itr != v.end(); ++itr)
	{
		if(name == NULL || *itr == name)
		{
			CMemQueue q;
			if(q.init(*itr, 0, 0))
			{
				cout << "destroying " << *itr << " ..." <<endl;
				q.printInfo();
				q.destroy();
				CMemoryQueueBase::instance()->deleteQueue(*itr);
				cout << "---" << endl;
			}
			else
			{
				cout << "err: can't init queue " << *itr << endl;
			}
		}
	}
}

void usage(const char *cmd)
{
	printf("Usage: %s [list|read|clean]\n"
			"\tread name\n"
			"\tclean [name], if not specify name, clean all queues\n", cmd);
	exit(-1);
}

int main(int argc, char* argv[])
{
	assert(init());

	if(argc < 2)
	{
		usage(argv[0]);
	}

	if(strcmp(argv[1], "list") == 0)
	{
		list();
	}
	else if(strcmp(argv[1], "read") == 0)
	{
		if(argc < 3)
		{
			printf("read command need io name. example: %s read testio\n", argv[0]);
			exit(-1);
		}
		assert(g_queue.init(argv[2], 0, 0));
		read();
	}
	else if(strcmp(argv[1], "clean") == 0)
	{
		if(argc == 3)
		{
			clean(argv[2]);
		}
		else
		{
			clean();
		}
	}
	else if(strcmp(argv[1], "test_read") == 0)
	{
		assert(g_queue.init("test_queue", 512, sizeof(stObj)));
		testRead();
	}
	else if(strcmp(argv[1], "test_write") == 0)
	{
		assert(g_queue.init("test_queue", 512, sizeof(stObj)));
		testWrite();
	}
	else
	{
		usage(argv[0]);
	}


	return 0;
}

