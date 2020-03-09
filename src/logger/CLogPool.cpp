/*
 * CLogPool.cpp
 *
 *  Created on: 2017年10月25日
 *      Author: hongxu
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "CLogPool.h"
#include "compiler.h"
#include "utils.h"


static string getDay()
{
	time_t now = time(NULL);
	struct tm result;
	localtime_r(&now, &result);

	char buf[64];
	snprintf(buf, sizeof(buf), "%d%02d%02d", result.tm_year + 1900,
			result.tm_mon + 1, result.tm_mday);

	string res = buf;
	return res;
}

CLogPool::CLogPool() : log_cnt_(0), is_need_update_day_(false), log_thread_pid_((pthread_t)-1), do_run_thread_(false), sleep_time_(10000)
{
	day_ = getDay();
}

CLogPool::~CLogPool()
{
	if(do_run_thread_)
	{
		do_run_thread_ = false;
		pthread_join(log_thread_pid_, NULL);
	}
	closeAll();
}

void CLogPool::closeAll()
{
	for(map<string, uint64_t>::iterator iter = filefdmap_.begin(); iter != filefdmap_.end(); ++iter)
	{
		int fd = (int)(iter->second & 0xffffffff);
		close(fd);
	}
	filefdmap_.clear();
}

CLogPool* CLogPool::instance()
{
	static CLogPool s_inst;
	return &s_inst;
}

void CLogPool::startLogThread()
{
	if(0 != pthread_create(&log_thread_pid_, NULL, runThread, this))
	{
		perror("create trader thread err");
	}
	else
	{
		do_run_thread_ = true;
	}
}

void* CLogPool::runThread(void *p_obj)
{
	((CLogPool*)p_obj)->threadLoop();
	return NULL;
}

int CLogPool::getNewLoggerID(string prefix, uint8_t mode, uint32_t bufsize)
{
	if(log_cnt_ >= MAX_LOGGER_CNT) return -1;
	if(!logger_[log_cnt_].init(prefix, mode, bufsize)) return -1;

	string file = prefix;
	if(mode == LOG_DAY)
	{
		file += day_ + ".log";
	}
	else file += ".log";

	map<string, uint64_t>::iterator iter = filefdmap_.find(file);
	if(iter == filefdmap_.end())
	{
		int fd = open(file.c_str(), O_CREAT|O_APPEND|O_CLOEXEC|O_RDWR, 0600);
		logger_[log_cnt_].setfd(fd);
		if(fd < 0) perror("open log file err");
		else
		{
			filefdmap_[file] = (1ULL << 32) | fd;
			if(mode == LOG_DAY) is_need_update_day_ = true;
		}
	}
	else
	{
		int fd = (int)(iter->second & 0xffffffff);
		int cnt = (int)(iter->second >> 32) + 1;
		iter->second = (((uint64_t)cnt) << 32) | fd;
		logger_[log_cnt_].setfd(fd);
	}

	return log_cnt_++;
}

void CLogPool::updateDayLogFile()
{
	if( ! is_need_update_day_ ) return;

	string nowday = getDay();
	if(nowday != day_)
	{
		map<string, uint64_t>::iterator iter;
		for(int i = 0; i < log_cnt_; ++i)
		{
			if(logger_[i].getMode() == LOG_DAY)
			{
				string file = logger_[i].getPrefix();
				file += nowday + ".log";

				iter = filefdmap_.find(file);
				if(iter == filefdmap_.end())
				{
					int fd = open(file.c_str(), O_CREAT|O_APPEND|O_CLOEXEC|O_RDWR, 0600);
					logger_[i].setfd(fd);
					if(fd < 0) perror("open log file err");
					else
					{
						filefdmap_[file] = (1ULL << 32) | fd;
					}
				}
				else
				{
					int fd = (int)(iter->second & 0xffffffff);
					int cnt = (int)(iter->second >> 32) + 1;
					iter->second = (((uint64_t)cnt) << 32) | fd;
					logger_[i].setfd(fd);
				}

				file = logger_[i].getPrefix();
				file += day_ + ".log";
				iter = filefdmap_.find(file);
				if(iter != filefdmap_.end())
				{
					int fd = (int)(iter->second & 0xffffffff);
					int cnt = (int)(iter->second >> 32) - 1;
					if(cnt > 0) iter->second = (((uint64_t)cnt) << 32) | fd;
					else
					{
						close(fd);
						filefdmap_.erase(iter);
					}
				}
			}
		}
		day_ = nowday;
	}
}

void CLogPool::threadLoop()
{
	setaffinity(LOG_THREAD_AFFINITY_CPU);

	do
	{
		updateDayLogFile();
		for(int i = 0; i < log_cnt_; ++i)
		{
			logger_[i].loopLogToFile();
		}

		if(sleep_time_) usleep(sleep_time_);

	}while(do_run_thread_);

	updateDayLogFile();
	for(int i = 0; i < log_cnt_; ++i)
	{
		logger_[i].loopLogToFile();
		logger_[i].flush();
	}
}

void CLogPool::setSleepTime(uint32_t usec)
{
	sleep_time_ = usec;
}

void CLogPool::doexit()
{
	if(do_run_thread_)
	{
		do_run_thread_ = false;
		pthread_join(log_thread_pid_, NULL);
	}
}

