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
                                                 is_test_lock_onload_(true) // try to lock the file on load, fail if can't lock
{

}

CRawIOWriter::~CRawIOWriter()
{

}

// Create next Page file.
// 'Next' means increment the page_num_ by 1 and use the new number as file suffix
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

// init this IOWriter with directory 'path'
// The file with largest existing Page_number will be loaded.
// If this file is already finished, increment the Page_number by 1 and create a new
// mmap Page file.
bool CRawIOWriter::init(string path)
{
  // Release resources, unmap the mmap file if opened
	unload();

  // Set the path prefix
	setPrefix(path);

  // Get the current largest number
  // This number corresponds to the latest created mmap file
	page_num_ = getTailNum();

  // Try to load the latest mmap file from disk and mapped into memory
  // Checking whether a new is needed based on current PAGE_STATUS.
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

  // Create a new mmap file if needed
	if(need_create && !createNextPage())
	{
		return false;
	}
	return true;
}

// Write data as a new frame into the Page.
// If the current Page's space is limited, close and create a new one.
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

// Instead of writing the data into new frame directly, fetch the empty frame first.
// 'len' indicates the size of the frame data bytes to be filled later.
// If the Page's space is limited, the current one will be closed and a new one created.
// 'prefetch_tail' will be set to the expected length of the valid data in the Page including the
// prefetched frame.
// The returned pointer can be used by the user to fill in the frame bytes later, directly.
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

// Used together with 'prefetch', the step where the frame data bytes are filled.
// The current timestamp will be used as the newly-filled frame time.
void CRawIOWriter::commit()
{
	tFrameHead *p_frame = (tFrameHead*)(buf_ + ((tPageHead*)buf_)->tail);
	p_frame->magic = FRAME_HEAD_MAGIC;
	p_frame->nano = CTimer::instance().getNano();
	((tPageHead*)buf_)->last_nano = p_frame->nano;
	((tPageHead*)buf_)->tail = prefetch_tail_;
}

// Load the Page with 'num'
bool CRawIOWriter::load(int num)
{
  // Compose the filename based on path prefix and 'num' as suffix
	string path = prefix_ + to_string(num);

  // Open the specified file, create it if not exist
	fd_ = open(path.c_str(), O_RDWR | O_CREAT, (mode_t)0666);
  if (fd_ < 0)
    {
      LOG_ERR("Cannot open file %s, err:%s", path.c_str(), strerror(errno));
      return false;
    }

  // Try to lock this file for exclusive write permission for this process.
  // Fail means conflicts in the usage of CRawIOWriter. The Pages under 'path'
  // prefix can only be filled in by one process at a time.
  // User can also turn off this behavior by setting 'is_test_lock_onload_'
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

  // Set the size of this Page file.
  // It's newly created set to default size, if not new, set the the actual size of file
  // The file is regarded as newly-created as long as it's size is smaller than the size
  // of PageHead.
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

  // Map the file into memory, so the data inside can be accessed
  buf_ = (char*)mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if ((void*)buf_ == MAP_FAILED)
    {
    	close(fd_); fd_ = -1;
      LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
      return false;
    }

  // Check the opened file is the correct mmap file by checking the buffer header,
  // if the file is not newly-created.
  if(!isnew && !checkPageHead(buf_))
    {
      munmap(buf_, size_);
      close(fd_); fd_ = -1;
    	LOG_ERR("file %s format err.", path.c_str());
    	return false;
    }

  // Try to avoid the page-out of the mapped-memory, i.e. lock the pages in physical memory
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
