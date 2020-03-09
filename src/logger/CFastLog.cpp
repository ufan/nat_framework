/*
 * CFastLog.cpp
 *
 *  Created on: 2017年11月15日
 *      Author: hongxu
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CFastLog.h"
#include "compiler.h"

format_func CFastLog::format_funcs_[CFastLog::LOG_TAG_END] = {NULL};

CFastLog::CFastLog() : is_block_write_(false), write_pos_(0), read_pos_(0), fd_(-1),
	buf_size_(0), buf_(nullptr), mode_(LOG_NONE)
{
	format_funcs_[LOG_STR_TAG] = formatStrFunc;
	format_funcs_[LOG_CPU_CYCLE] = formatCycleFunc;
}

CFastLog::~CFastLog()
{
	if(buf_)
	{
		delete [] buf_;
	}
}

bool CFastLog::init(string prefix, uint8_t mode, uint32_t bufsize)
{
	mode_ 	= mode;
	prefix_ = prefix;

	buf_ = new char[bufsize];
	buf_size_ = bufsize - 1;		// reserve one for redundancy
	return true;
}

void CFastLog::setBlockWriteMode(bool is_set)
{
	is_block_write_ = is_set;
}

uint32_t CFastLog::write(uint8_t tag, const char* p, uint32_t len)
{
	if(!is_block_write_)
	{
		uint32_t wpos = write_pos_;
		if(read_pos_ <= wpos)
		{
			uint32_t left = read_pos_ != 0 ? buf_size_ - wpos : buf_size_ - wpos - 1;
			if(unlikely(left <= len))
			{
				left = read_pos_ != 0 ? read_pos_ - 1 : 0;
				if(unlikely(left <= len)) return 0;
				buf_[wpos] = LOG_FROM_HEAD;
				wpos = 0;
			}
		}
		else
		{
			uint32_t left = read_pos_ - wpos - 1;
			if(unlikely(left <= len)) return 0;
		}

		buf_[wpos++] = tag;
		memcpy(buf_ + wpos, p, len);
		write_pos_ = wpos + len;
		return len;
	}
	else
	{
		string log;
		format_func f = format_funcs_[tag];
		f(p, log);
		int ret = ::write(fd_, log.data(), log.size());
		if(ret < 0)
		{
			perror("log pool write err.");
			return 0;
		}
		return ret;
	}
}

const char* CFastLog::read()
{
	if(read_pos_ != write_pos_)
	{
		if(buf_[read_pos_] == LOG_FROM_HEAD) read_pos_ = 0;
		return buf_ + read_pos_;
	}
	return NULL;
}

void CFastLog::loopLogToFile()
{
	if(fd_ >= 0)
	{
		string log;
		const char *p = NULL;
		while((p = read()) != NULL)
		{
			format_func f = format_funcs_[*(uint8_t*)p++];
			int ret = f(p, log);
			commitRead(ret + 1);		// 1 for tag

			ret = ::write(fd_, log.data(), log.size());
			if(ret < 0)
			{
				perror("log pool write err");
				return;
			}
		}
	}
}

void CFastLog::flush()
{
	if(fd_ >= 0)
	{
		fsync(fd_);
	}
}

int CFastLog::formatStrFunc(const void* src, string& res)
{
	res = (const char *)src;
	return res.size() + 1; 		// 1 for \0
}

int CFastLog::formatCycleFunc(const void* src, string& res)
{
	uint64_t t = *(uint64_t*)src;
	char buf[50];
	snprintf(buf, sizeof(buf), "cpu cycle: %llu\n", (unsigned long long)t);
	res = buf;
	return sizeof(t);
}

