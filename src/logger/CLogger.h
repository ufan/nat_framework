/*
 * CLogger.h
 *
 *  Created on: 2017年11月2日
 *      Author: hongxu
 */

#ifndef SRC_LOGGER_CLOGGER_H_
#define SRC_LOGGER_CLOGGER_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
using namespace std;

enum emLOGMODE
{
	LOG_NONE,
	LOG_DAY,
};

class CLogger
{
public:
	CLogger();
	virtual ~CLogger();

	bool init(string prefix, uint8_t mode, uint32_t bufsize=4096);

	uint32_t write(const char *p, uint32_t len);

	const char* read(uint32_t &len)
	{
		len = read_pos_ <= write_pos_ ? write_pos_ - read_pos_ : buf_size_ - read_pos_;
		return buf_ + read_pos_;
	}

	void commitRead(uint32_t len)
	{
		read_pos_ = (read_pos_ + len) % buf_size_;
	}

	void log(const char* p)
	{
		write(p, strlen(p));
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
			if(ret < sizeof(buf)) write(buf, ret);
			else write(buf, sizeof(buf));
		}
	}

	void flush();

	void loopLogToFile();

	void setfd(int fd) {fd_ = fd;}

	string getPrefix() {return prefix_;}

	uint8_t getMode() {return mode_;}

private:
	uint32_t 		write_pos_;
	uint32_t		read_pos_;
	int				fd_;
	uint32_t		buf_size_;
	char 			*buf_;

	uint8_t 		mode_;
	string 			prefix_;
};

#endif /* SRC_LOGGER_CLOGGER_H_ */
