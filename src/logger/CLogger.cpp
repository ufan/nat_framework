/*
 * CLogger.cpp
 *
 *  Created on: 2017年11月2日
 *      Author: hongxu
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CLogger.h"
#include "compiler.h"


CLogger::CLogger() : write_pos_(0), read_pos_(0), fd_(-1),
	buf_size_(0), buf_(NULL), mode_(LOG_NONE)
{

}

CLogger::~CLogger()
{
	if(buf_)
	{
		delete [] buf_;
	}
}

bool CLogger::init(string prefix, uint8_t mode, uint32_t bufsize)
{
	mode_ 	= mode;
	prefix_ = prefix;

	buf_ = new char[bufsize];
	buf_size_ = bufsize;
	return true;
}

uint32_t CLogger::write(const char* p, uint32_t len)
{
	if(read_pos_ <= write_pos_)
	{
		uint32_t left_cap = buf_size_ + read_pos_ - write_pos_ - 1;
		len = len < left_cap ? len : left_cap;
		if(len > 0)
		{
			uint32_t left = buf_size_ - write_pos_;
			if(left >= len)
			{
				memcpy(buf_ + write_pos_, p, len);
				write_pos_ = left > len ? write_pos_ + len : 0;
			}
			else
			{
				memcpy(buf_ + write_pos_, p, left);
				memcpy(buf_, p + left, len - left);
				write_pos_ = len - left;
			}
		}
	}
	else
	{
		uint32_t left_cap = read_pos_ - write_pos_ - 1;
		len = len < left_cap ? len : left_cap;
		memcpy(buf_ + write_pos_, p, len);
		write_pos_ += len;
	}
	return len;
}

void CLogger::loopLogToFile()
{
	if(fd_ >= 0)
	{
		uint32_t len;
		const char *p = read(len);
		while(len > 0)
		{
			int ret = ::write(fd_, p, len);
			if(ret < 0)
			{
				perror("log pool write err");
				return;
			}
			commitRead(len);
			p = read(len);
		}
	}
}

void CLogger::flush()
{
	if(fd_ >= 0)
	{
		fsync(fd_);
	}
}

