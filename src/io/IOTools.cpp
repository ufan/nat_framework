/*
 * IOTools.cpp
 *
 *  Created on: 2018年4月25日
 *      Author: hongxu
 */

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include "CRawIOReader.h"
#include "CRawIOWriter.h"
#include "CReaderPool.h"
#include "CTimer.h"
using namespace std;


void printBuf(const char *buf, uint32_t len)
{
	printf("{\n");
	for(uint32_t i = 0; i < len; i++)
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
	printf("\n}\n");
}

void parseBuf(const char *buf, uint32_t len, const char *desc)
{
	printf("{\n");
	uint32_t l = 0;
	char val[8];
	const char *end = buf + len;
	while(buf < end && *desc != '\0')
	{
		len = (uint32_t)(end - buf);
		switch(*desc++)
		{
		case 'i':
			l = strtoul(desc, (char**)&desc, 10); l=l<len?l:len; assert(l <= 8);
			memset(val, 0, sizeof(val)); memcpy(&val, buf, l);
			printf("i: %lld\n", *(long long*)val);
			buf += l;
			break;

		case 'u':
			l = strtoul(desc, (char**)&desc, 10); l=l<len?l:len; assert(l <= 8);
			memset(val, 0, sizeof(val)); memcpy(&val, buf, l);
			printf("u: %llu\n", *(unsigned long long*)val);
			buf += l;
			break;

		case 'd':
			l = strtoul(desc, (char**)&desc, 10); l=l<len?l:len; assert(l <= 8);
			memset(val, 0, sizeof(val)); memcpy(&val, buf, l);
			if(l <= 4) printf("d: %f\n", *(float*)val);
			else printf("d: %lf\n", *(double*)val);
			buf += l;
			break;

		case 's':
			l = strlen(buf);
			if(buf + l >= end)
			{
				printf("s: %.*s\n", len, buf);
				buf = end;
			}
			else
			{
				printf("s: %s\n", buf);
				buf += l;
			}
			break;

		case 'c':
			l = strtoul(desc, (char**)&desc, 10);
			printf("c: ");
			for(uint32_t i = 0; i < l && buf < end; i++, buf++)
			{
				if(isprint(*buf))
				{
					putchar(*buf);
				}
				else
				{
					printf("\\%02x", (unsigned char)*buf);
				}
			}
			printf("\n");
			break;

		case 'x':
			l = strtoul(desc, (char**)&desc, 10);
			printf("x: ");
			for(uint32_t i = 0; i < l && buf < end; i++)
			{
				printf("0x%02x ", (unsigned char)*buf++);
			}
			printf("\n");
			break;

		default:
			cerr << "describe string \"" << desc << "\" format err." << endl;
			printf("}\n");
			return;
		}
	}

	if(buf < end)
	{
		printf("left: ");
		while(buf < end)
		{
			if(isprint(*buf))
			{
				putchar(*buf);
			}
			else
			{
				printf("\\%02x", (unsigned char)*buf);
			}
			buf++;
		}
		printf("\n");
	}

	printf("}\n");
}

void readIO(const char *file, int from_no, long from_nano, const char *desc=NULL)
{
	CReaderPool pool;
	pool.add(666666, file, from_no, from_nano);
	const char *p = NULL;
	uint32_t len = 0;
	uint32_t hash = 0;
	while(true)
	{
		p = pool.seqRead(len, hash);
		if(p)
		{
			long nano = getIOFrameHead(p)->nano;
			string t = parseNano(nano, "%Y%m%d %H:%M:%S");
			LOG_DBG("addr:%lx|nano:%ld|%s|read len:%d", (uint64_t)p, nano, t.c_str(), len);
			if(!desc) printBuf(p, len);
			else parseBuf(p, len, desc);
		}
		else
		{
			usleep(50000);
		}
	}
}


void writeTimer(const char *file)
{
	CRawIOWriter writer(4096);
	assert(writer.init(file));
	while(true)
	{
		long nano = CTimer::instance().getNano();
		writer.write(&nano, sizeof(nano));
		usleep(1000000);
	}
}

void writeFile(const char *file, const char *from)
{
	ifstream in(from);
	if(!in)
	{
		cerr << "file " << from << " not exists." << endl;
		return;
	}

	CRawIOWriter writer(4096);
	assert(writer.init(file));
	string line;
	while(getline(in, line))
	{
		char *p = writer.prefetch(line.size());
		memcpy(p, line.data(), line.size());
		writer.commit();
	}

	in.close();
}

void writeInput(const char *file)
{
	CRawIOWriter writer(4096);
	assert(writer.init(file));
	string line;
	while(getline(cin, line))
	{
		char *p = writer.prefetch(line.size());
		memcpy(p, line.data(), line.size());
		writer.commit();
	}
}

int main(int argc, char *argv[])
{
	initLogger("./log.cnf");

	if(argc < 3)
	{
		printf("Usage: %s read file [from_no from_nano desc] | write file [timer|file in_file]\n"
				"\t desc: i,u,d,c,x,s\n", argv[0]);
		return -1;
	}

	if(strcasecmp(argv[1], "read") == 0)
	{
		int from_no = -1;
		long from_nano = -1;
		const char *desc = NULL;
		if(argc == 4)
		{
			from_no = atoi(argv[3]);
		}
		else if(argc > 4)
		{
			from_no = atoi(argv[3]);
			from_nano = stol(string(argv[4]));
			if(argc > 5) desc = argv[5];
		}
		readIO(argv[2], from_no, from_nano, desc);
	}
	else if(strcasecmp(argv[1], "write") == 0)
	{
		if(argc >= 4)
		{
			if(strcasecmp(argv[3], "timer") == 0)
			{
				writeTimer(argv[2]);
			}
			else if(strcasecmp(argv[3], "file") == 0)
			{
				if(argc < 5)
				{
					cerr << "need input file" << endl;
					return -1;
				}
				writeFile(argv[2], argv[4]);
			}
		}
		else writeInput(argv[2]);
	}

	return 0;
}
