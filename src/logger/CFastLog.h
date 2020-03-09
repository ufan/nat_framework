/*
 * CFastLog.h
 *
 *  Created on: 2017年11月15日
 *      Author: hongxu
 */

#ifndef LIB_LOG_CFASTLOG_H_
#define LIB_LOG_CFASTLOG_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "utils.h"
#include "compiler.h"
using namespace std;

enum emLOGMODE
{
	LOG_NONE,
	LOG_DAY,
};

typedef int (*format_func)(const void* src, string &res);

class CFastLog
{
public:
	static const uint32_t MAX_FORMAT_FUNC = 20;

	enum emReservedLogTag
	{
		LOG_FROM_HEAD = MAX_FORMAT_FUNC,
		LOG_STR_TAG,
		LOG_CPU_CYCLE,

		LOG_TAG_END,
	};

public:
	CFastLog();
	virtual ~CFastLog();

	bool init(string prefix, uint8_t mode, uint32_t bufsize=4096);

	uint32_t write(uint8_t tag, const char *p, uint32_t len);

	const char* read();

	void commitRead(uint32_t len)
	{
		read_pos_ += len;
	}

	void logFmt(const char *format, ...)
	{
		char buf[1024];
		va_list ap;
		va_start(ap, format);
		int ret = vsnprintf(buf, sizeof(buf), format, ap);
		va_end(ap);

		if(ret > 0)
		{
			if(buf_)
			{
				if(ret < sizeof(buf)) write(LOG_STR_TAG, buf, ret + 1);
				else
				{
					buf[sizeof(buf) - 1] = '\0';
					write(LOG_STR_TAG, buf, sizeof(buf));
				}
			}
			else
			{
				if(ret >= sizeof(buf)) buf[sizeof(buf) - 1] = '\0';
				fputs(buf, stdout);
			}
		}
	}

	void logCycle()
	{
		uint32_t h,l;
		getCycleBegin(h,l);
		uint64_t c = getu64(h, l);
		write(LOG_CPU_CYCLE, (const char *)&c, sizeof(c));
	}

	void flush();

	void loopLogToFile();

	void setfd(int fd) {fd_ = fd;}

	string getPrefix() {return prefix_;}

	uint8_t getMode() {return mode_;}

	void setBlockWriteMode(bool);

	static bool registerFormater(int id, format_func func)
	{
		if(id >= MAX_FORMAT_FUNC) return false;
		format_funcs_[id] = func;
		return true;
	}

	static int formatStrFunc(const void* src, string &res);
	static int formatCycleFunc(const void* src, string &res);

private:
	bool 					is_block_write_;
	volatile uint32_t 		write_pos_;
	volatile uint32_t		read_pos_;
	int				fd_;
	uint32_t		buf_size_;
	char 			*buf_;

	uint8_t 		mode_;
	string 			prefix_;

	static format_func format_funcs_[LOG_TAG_END];
};

#endif /* LIB_LOG_CFASTLOG_H_ */
