/*
 * test.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: hongxu
 */
#include <assert.h>
#include <unistd.h>
#include "CLogPool.h"
#include "Logger.h"


bool test()
{
	int id = CLogPool::instance()->getNewLoggerID("./test", LOG_DAY);
	CLogPool::instance()->setSleepTime(0);
	CLogPool::instance()->startLogThread();

	LOG_CYCLE(0);

	for(uint32_t i = 0; i < 8000; i++)
	{
		LOG0("test for %d and %s", 123, "hello");
		LOG0("test for %d and %s", 123, "hello");
		LOG0("test for %d and %s", 123, "hello");
		usleep(1000);
	}

	LOG_CYCLE(0);

	return true;
}


int main()
{
	test();
	return 0;
}



