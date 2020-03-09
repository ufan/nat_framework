/*
 * EngLogger.cpp
 *
 *  Created on: May 22, 2018
 *      Author: hongxu
 */

#include <string>
#include "SysConf.h"
#include "CLogPool.h"
#include "FastLogger.h"
using namespace std;

int g_englogger_id_ = 0;
int g_fastlogger_level_ = FASTLOG_INFO;

static uint32_t s_buf_size = 2*4096;
static uint8_t s_file_mode = LOG_DAY;
static bool	s_block_mode = false;
static bool s_is_not_write = false;

int getFastLogLevel(string level)
{
    if(level == "DBG") return FASTLOG_DBG;
    else if(level == "INFO") return FASTLOG_INFO;
    else if(level == "WARN") return FASTLOG_WARN;
    else if(level == "ERR") return FASTLOG_ERR;
    else if(level == "TRACE") return FASTLOG_TRACE;
    else if(isdigit(level[0])) return atoi(level.c_str());
    else return FASTLOG_INFO;
}

void initFastLogger(const json &j_conf)
{
	static bool s_has_init = false;
	if(!s_has_init)
	{
		if(j_conf.find("Logger") == j_conf.end()) return;
		auto& log_conf = j_conf["Logger"];

        if(log_conf.find("level") != log_conf.end())
        {
            g_fastlogger_level_ = getFastLogLevel(log_conf["level"]);
        }

		string write_mode = log_conf["write_mode"];
		if(write_mode == "none")
		{
			s_is_not_write = true;
			return;
		}

		string name = j_conf["name"];
		string path = string(STRATEGY_LOG_BASE_DIR) + name;
		if(log_conf.find("path") != log_conf.end()) path = log_conf["path"].get<string>() + name;

		s_buf_size = log_conf["cycle_buffer_kb"];
		s_buf_size *= 1024;
		s_file_mode = log_conf["file_mode"].get<string>() == "DAY" ? LOG_DAY : LOG_NONE;

		g_englogger_id_ = CLogPool::instance()->getNewLoggerID(path, s_file_mode, s_buf_size);

		if(write_mode == "thread")
		{
			CLogPool::instance()->setSleepTime(log_conf["thread_cycle_us"]);
			CLogPool::instance()->startLogThread();
		}
		else if(write_mode == "block")
		{
			CLogPool::instance()->getLogger(g_englogger_id_).setBlockWriteMode(true);
			s_block_mode = true;
		}

		s_has_init = true;
	}
}

void initFastLoggerConf(int buf_size_kb, string file_mode, string write_mode, int thread_cycle_us)
{
	static bool s_has_init = false;
	if(!s_has_init)
	{
		if(write_mode == "none")
		{
			s_is_not_write = true;
			return;
		}


		s_buf_size = buf_size_kb * 1024;
		s_file_mode = file_mode == "DAY" ? LOG_DAY : LOG_NONE;

		if(write_mode == "thread")
		{
			CLogPool::instance()->setSleepTime(thread_cycle_us);
			CLogPool::instance()->startLogThread();
		}
		else if(write_mode == "block")
		{
			s_block_mode = true;
		}

		s_has_init = true;
	}
}

int getNewLogger(string path)
{
	if(not s_is_not_write)
	{
		int id = CLogPool::instance()->getNewLoggerID(path, s_file_mode, s_buf_size);
		if(s_block_mode) CLogPool::instance()->getLogger(id).setBlockWriteMode(true);
		return id;
	}
	return 0;
}

void fastLog(int id, string log)
{
	CLogPool::instance()->getLogger(id).logFmt("%s\n", log.c_str());
}

int getStgLoggerId() {return g_englogger_id_;}
