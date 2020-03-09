/*
 * testmdlatency.cpp
 *
 *  Created on: 2018年11月19日
 *      Author: hongxu
 */

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "Logger.h"
#include "CRawIOReader.h"
#include "CRawIOWriter.h"
#include "CTimer.h"
#include "ATStructure.h"
#include "IOCommon.h"

const char *path = "./mdtest";

void iowrite()
{
	srandom((unsigned int)time(nullptr));

	CRawIOWriter w;
	w.init(path);
	while(true)
	{
		usleep(random() % 10000000);	// 10s
		tIOMarketData data;
		w.write(&data, sizeof(data));
	}
}

void ioread()
{
	CRawIOReader r;
	r.init(path, -1, -1);
	while(true)
	{
		uint32_t len = 0;
		const char *p = r.read(len);
		if(p && *(int*)p == IO_MARKET_DATA)
		{
			long nano = CTimer::instance().getNano();
			nano -= getIOFrameHead(p)->nano;
			printf("%ld\n", nano);
		}
	}
}


int main(int argc, char *argv[])
{
	initLogger("./logger.cnf");

	if(argc == 1)
	{
		iowrite();
	}
	else ioread();
	return 0;
}
