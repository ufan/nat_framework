/*
 * CRawIOWriter.cpp
 *
 *  Created on: 2018年4月23日
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
#include "CGlobalLock.h"
#include "MurmurHash2.h"
#include "CRawIOWriter.h"
#include "utils.h"


CRawIOWriter::CRawIOWriter(uint32_t page_size) : page_num_(-1), prefetch_tail_(0), page_size_(page_size),
	is_test_lock_onload_(true)
{

}

CRawIOWriter::~CRawIOWriter()
{

}

bool CRawIOWriter::createNextPage()
{
	if(unlikely(!load(page_num_ + 1)))
	{
		LOG_ERR("writer load err");
		return false;
	}
	++page_num_;

	// fill head
	((tPageHead*)buf_)->status = PAGE_STATUS_WRITTING;
	((tPageHead*)buf_)->size = size_;
	((tPageHead*)buf_)->tail = sizeof(tPageHead);
	((tPageHead*)buf_)->start_nano = CTimer::instance().getNano();
	((tPageHead*)buf_)->stop_nano = -1;
	((tPageHead*)buf_)->reserved_ = 0;
	((tPageHead*)buf_)->version = PAGE_HEAD_VER_1;

	return true;
}

bool CRawIOWriter::init(string path)
{
	unload();
	setPrefix(path);
	page_num_ = getTailNum();
	bool need_create = false;
	if(page_num_ >= 0)
	{
		if(!load(page_num_))
		{
			LOG_ERR("writer load err");
			return false;
		}
		if(((tPageHead*)buf_)->status == PAGE_STATUS_FINISH)
		{
			unload();
			need_create = true;
		}
	}
	else need_create = true;

	if(need_create && !createNextPage())
	{
		return false;
	}
	return true;
}

bool CRawIOWriter::write(const void* data, uint32_t len)
{
	uint32_t tail = ((tPageHead*)buf_)->tail + sizeof(tFrameHead) + len;
	if(tail <= size_)
	{
		tFrameHead *p_frame = (tFrameHead*)(buf_ + ((tPageHead*)buf_)->tail);
		memcpy(p_frame->buf, data, len);
		p_frame->len = len;
		p_frame->magic = FRAME_HEAD_MAGIC;
		p_frame->nano = CTimer::instance().getNano();
		((tPageHead*)buf_)->last_nano = p_frame->nano;
		((tPageHead*)buf_)->tail = tail;
		return true;
	}
	else
	{
		((tPageHead*)buf_)->stop_nano = CTimer::instance().getNano();
		((tPageHead*)buf_)->status = PAGE_STATUS_FINISH;

		unload();
		if(!createNextPage())
		{
			LOG_ERR("create new page err.");
			return false;
		}

		if(len <= size_ - sizeof(tPageHead) - sizeof(tFrameHead))
		{
			return write(data, len);
		}
	}
	return false;
}

char* CRawIOWriter::prefetch(uint32_t len)
{
	prefetch_tail_ = ((tPageHead*)buf_)->tail + sizeof(tFrameHead) + len;
	if(prefetch_tail_ <= size_)
	{
		tFrameHead *p_frame = (tFrameHead*)(buf_ + ((tPageHead*)buf_)->tail);
		p_frame->len = len;
		return p_frame->buf;
	}
	else
	{
		((tPageHead*)buf_)->stop_nano = CTimer::instance().getNano();
		((tPageHead*)buf_)->status = PAGE_STATUS_FINISH;

		unload();
		if(!createNextPage())
		{
			LOG_ERR("create new page err.");
			return NULL;
		}

		prefetch_tail_ = ((tPageHead*)buf_)->tail + sizeof(tFrameHead) + len;
		if(prefetch_tail_ <= size_)
		{
			tFrameHead *p_frame = (tFrameHead*)(buf_ + ((tPageHead*)buf_)->tail);
			p_frame->len = len;
			return p_frame->buf;
		}
	}
	return NULL;
}

void CRawIOWriter::commit()
{
	tFrameHead *p_frame = (tFrameHead*)(buf_ + ((tPageHead*)buf_)->tail);
	p_frame->magic = FRAME_HEAD_MAGIC;
	p_frame->nano = CTimer::instance().getNano();
	((tPageHead*)buf_)->last_nano = p_frame->nano;
	((tPageHead*)buf_)->tail = prefetch_tail_;
}

bool CRawIOWriter::load(int num)
{
	string path = prefix_ + to_string(num);

	fd_ = open(path.c_str(), O_RDWR | O_CREAT, (mode_t)0666);
    if (fd_ < 0)
    {
        LOG_ERR("Cannot open file %s, err:%s", path.c_str(), strerror(errno));
        return false;
    }

	if(is_test_lock_onload_ && !tryFileLock(fd_, 0))
	{
		LOG_ERR("file %s already has a writer.", path.c_str());
		close(fd_); fd_ = -1;
		return false;
	}

	struct stat statbuff;
	if(fstat(fd_, &statbuff) < 0)
	{
		close(fd_); fd_ = -1;
		LOG_ERR("stat file %s fail, err:%s", path.c_str(), strerror(errno));
		return false;
	}

	bool isnew = false;
	if(statbuff.st_size < sizeof(tPageHead))	// a new file
	{
		isnew = true;
		size_ = page_size_;
		if(ftruncate(fd_, size_) < 0)
		{
			close(fd_); fd_ = -1;
			LOG_ERR("truncate file %s fail, err:%s", path.c_str(), strerror(errno));
			return false;
		}
	}
	else size_ = statbuff.st_size;

    buf_ = (char*)mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if ((void*)buf_ == MAP_FAILED)
    {
    	close(fd_); fd_ = -1;
        LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
        return false;
    }

    if(!isnew && !checkPageHead(buf_))
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

    return true;
}

atomic_flag 	g_io_spin_flag = ATOMIC_FLAG_INIT;

bool CSafeRawIOWriter::init(string path)
{
	while (g_io_spin_flag.test_and_set(memory_order_acquire));
	bool ret = CRawIOWriter::init(path);
	g_io_spin_flag.clear(memory_order_release);
	return ret;
}

bool CSafeRawIOWriter::write(const void* data, uint32_t len)
{
	while (g_io_spin_flag.test_and_set(memory_order_acquire));
	bool ret = CRawIOWriter::write(data, len);
	g_io_spin_flag.clear(memory_order_release);
	return ret;
}

char* CSafeRawIOWriter::prefetch(uint32_t len)
{
	while (g_io_spin_flag.test_and_set(memory_order_acquire));
	char* ret = CRawIOWriter::prefetch(len);
	return ret;
}

void CSafeRawIOWriter::commit()
{
	CRawIOWriter::commit();
	g_io_spin_flag.clear(memory_order_release);
}

void CSafeRawIOWriter::discard()
{
	g_io_spin_flag.clear(memory_order_release);
}
