/*
 * CLogPool.h
 *
 *  Created on: 2017年10月25日
 *      Author: hongxu
 */

#ifndef SRC_LOGGER_CLOGPOOL_H_
#define SRC_LOGGER_CLOGPOOL_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <map>
#include <string>
#include "CFastLog.h"
using namespace std;

#define LOG(id, format, ...) do{ CLogPool::instance()->getLogger(id).logFmt(format"\n", ##__VA_ARGS__); }while(0)
#define LOG0(format, ...) LOG(0, format, ##__VA_ARGS__)
#define LOG_CYCLE(id) do{ CLogPool::instance()->getLogger(id).logCycle(); }while(0)
#define FASTLOG(id, tag, p, len) do{ CLogPool::instance()->getLogger(id).write((uint8_t)(tag), (const char*)(p), (uint32_t)(len)); }while(0)

#define LOG_THREAD_AFFINITY_CPU 1


class CLogPool
{
	static const uint32_t MAX_LOGGER_CNT = 32;
private:
	CLogPool();
public:
	virtual ~CLogPool();

	int getNewLoggerID(string prefix, uint8_t mode, uint32_t bufsize=2*4096);

	CFastLog& getLogger(int id) {return logger_[id];}

	void startLogThread();

	static CLogPool* instance();

	void setSleepTime(uint32_t usec);

	void doexit();

private:
	static void* runThread(void *p_obj);

	void threadLoop();

	void closeAll();

	void updateDayLogFile();

private:
	CFastLog			logger_[MAX_LOGGER_CNT];
	int					log_cnt_;

	bool				is_need_update_day_;

	map<string, uint64_t>	filefdmap_;

	pthread_t			log_thread_pid_;
	volatile bool		do_run_thread_;

	uint32_t			sleep_time_;
	string 				day_;
};


#endif /* SRC_LOGGER_CLOGPOOL_H_ */

