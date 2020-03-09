/*
 * CRawIOReader.cpp
 *
 *  Created on: 2018年4月24日
 *      Author: hongxu
 */

#include <sys/mman.h>
#include <glob.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include "CRawIOReader.h"

CRawIOReader::CRawIOReader() : cursor_(NULL), page_num_(-1)
{

}

CRawIOReader::~CRawIOReader()
{

}

bool CRawIOReader::init(string path, long from_nano)
{
	setPrefix(path);
	if(from_nano < 0) return init(path, -1, from_nano);
	else return init(path, getHeadNum(), from_nano);
}

bool CRawIOReader::init(string path, int from_page, long from_nano)
{
	unload();
	setPrefix(path);
	if(from_page < 0)
	{
		from_page = getTailNum();
	}
	page_num_ = from_page < 0 ? 0 : from_page;
	if(load(page_num_))
	{
		seekTime(from_nano);
	}
	return true;
}

void CRawIOReader::seek(int frame_no)
{
	if(frame_no < 0)
	{
		cursor_ = buf_ + ((tPageHead*)buf_)->tail;
	}
	else
	{
		cursor_ = ((tPageHead*)buf_)->buf;
		char *end = buf_ + ((tPageHead*)buf_)->tail;
		while(frame_no > 0 && cursor_ < end)
		{
			cursor_ += ((tFrameHead*)cursor_)->len + sizeof(tFrameHead);
			--frame_no;
		}
	}
}

inline bool CRawIOReader::passPage()
{
	if(fd_ < 0)
	{
		return load(page_num_);
	}
	else if(((volatile tPageHead* volatile)buf_)->status == PAGE_STATUS_FINISH)
	{
		unload();
		return load(++page_num_);
	}
	return false;
}

void CRawIOReader::seekTime(long nano)
{
	if(nano < 0)
	{
		cursor_ = buf_ + ((tPageHead*)buf_)->tail;
		return;
	}

	while(((volatile tPageHead* volatile)buf_)->last_nano < nano)
	{
		if(!passPage())
		{
			if(fd_ >= 0)
			{
				cursor_ = buf_ + ((tPageHead*)buf_)->tail;
			}
			return;
		}
	}

	while(((tFrameHead*)cursor_)->nano < nano)
	{
		cursor_ += ((tFrameHead*)cursor_)->len + sizeof(tFrameHead);
	}
}

void CRawIOReader::setReadPos(long nano)
{
	if(fd_ >= 0)
	{
		if(((tPageHead* volatile)buf_)->last_nano <= nano)
		{
			seekTime(nano);
			return;
		}
		else
		{
			unload();
		}
	}

	int from_page = getHeadNum();
	page_num_ = from_page < 0 ? 0 : from_page;
	if(load(page_num_))
	{
		seekTime(nano);
	}
}

bool CRawIOReader::load(int num)
{
	string path = prefix_ + to_string(num);

	fd_ = open(path.c_str(), O_RDONLY, (mode_t)0666);
    if (fd_ < 0)
    {
        if(unlikely(errno != ENOENT))
        {
        	LOG_ERR("Cannot open file %s, err %d:%s", path.c_str(), errno, strerror(errno));
        }
        return false;
    }

	struct stat statbuff;
	if(fstat(fd_, &statbuff) < 0)
	{
		close(fd_); fd_ = -1;
		LOG_ERR("stat file %s fail, err %d:%s", path.c_str(), errno, strerror(errno));
		return false;
	}
	size_ = statbuff.st_size;
	if(unlikely(size_ <  sizeof(tPageHead)))
	{
		close(fd_); fd_ = -1;
		LOG_ERR("file %s size too small.", path.c_str());
		return false;
	}

    buf_ = (char*)mmap(0, size_, PROT_READ, MAP_SHARED, fd_, 0);
    if ((void*)buf_ == MAP_FAILED)
    {
    	close(fd_); fd_ = -1;
        LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
        return false;
    }

    if(!checkPageHead(buf_))
    {
        munmap(buf_, size_);
        close(fd_); fd_ = -1;
    	LOG_ERR("file %s format err.", path.c_str());
    	return false;
    }

    if (is_lock_ && (madvise(buf_, size_, MADV_SEQUENTIAL) != 0 || mlock(buf_, size_) != 0))
    {
        //munmap(buf_, size_);
        //close(fd_); fd_ = -1;
        LOG_WARN("madvise or mlock error. abandon lock memory");
        //return false;
    }

    cursor_ = ((tPageHead*)buf_)->buf;
    return true;
}

