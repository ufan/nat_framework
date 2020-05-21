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

// Find the correct the Page and mapped into memory for reading
// Default is the latest one, if 'from_nano' is negative
// 'from_nano' defines the first frame to be read from,
// of which the filling time is just after 'from_nano'
bool CRawIOReader::init(string path, long from_nano)
{
	setPrefix(path);
	if(from_nano < 0) return init(path, -1, from_nano);
	else return init(path, getHeadNum(), from_nano);
}

bool CRawIOReader::init(string path, int from_page, long from_nano)
{
  // unload current Page file
	unload();

  // Set path prefix
	setPrefix(path);

  // Start from the latest Page file if 'from_page' is negative
	if(from_page < 0)
	{
		from_page = getTailNum();
	}

  // load the latest Page file or if no Page file exist
	page_num_ = from_page < 0 ? 0 : from_page;
	if(load(page_num_))
	{
		seekTime(from_nano);
	}
	return true;
}

// Move cursor to the frame_noth frame in current Page
// frame_no < 0 means moving to the tail of current Page
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

// Load the next finished Page file
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

// Find the frame just after time 'nano'
// If no frame filled after 'nano', just move the cursor to the tail of the current Page
// Internal use in class, can't handle the situation when nano is very small even for current Page
// Use setReadPos to correctly find the frame in all situations
void CRawIOReader::seekTime(long nano)
{
  // If invalid argument passedin, move cursor to the tail of current Page
	if(nano < 0)
	{
		cursor_ = buf_ + ((tPageHead*)buf_)->tail;
		return;
	}

  // Find the Page file which may contain frame before timestamp 'nano'
  // If not, then set to current Page's tail
	while(((volatile tPageHead* volatile)buf_)->last_nano < nano)
	{
    // If it's the current unfinished Page file, move cursor the the tail
		if(!passPage())
		{
			if(fd_ >= 0)
			{
				cursor_ = buf_ + ((tPageHead*)buf_)->tail;
			}
			return;
		}
	}

  // A previous Page file may contain the frame now
  // Move the cursor to the first fame which was filled after 'nano'
	while(((tFrameHead*)cursor_)->nano < nano)
	{
		cursor_ += ((tFrameHead*)cursor_)->len + sizeof(tFrameHead);
	}
}

// Set the Page file and cursor to the frame which is just after 'nano'
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

// Load existing Page file with 'num' as suffix
bool CRawIOReader::load(int num)
{
  // Compose the filename
	string path = prefix_ + to_string(num);

  // Open the file in read-only mode.
  // The file must exist otherwise fail.
	fd_ = open(path.c_str(), O_RDONLY, (mode_t)0666);
  if (fd_ < 0)
    {
      if(unlikely(errno != ENOENT))
        {
        	LOG_ERR("Cannot open file %s, err %d:%s", path.c_str(), errno, strerror(errno));
        }
      return false;
    }

  // Check the size of opened Page file.
  // The file must have the correct format by checking
  // whether the capacity is at least larger than PageHead
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

  // Map the file into memory
  buf_ = (char*)mmap(0, size_, PROT_READ, MAP_SHARED, fd_, 0);
  if ((void*)buf_ == MAP_FAILED)
    {
    	close(fd_); fd_ = -1;
      LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
      return false;
    }

  // Check the format by checking PageHead
  if(!checkPageHead(buf_))
    {
      munmap(buf_, size_);
      close(fd_); fd_ = -1;
    	LOG_ERR("file %s format err.", path.c_str());
    	return false;
    }

  // Try to avoid the page-out of the mapped memory
  if (is_lock_ && (madvise(buf_, size_, MADV_SEQUENTIAL) != 0 || mlock(buf_, size_) != 0))
    {
      //munmap(buf_, size_);
      //close(fd_); fd_ = -1;
      LOG_WARN("madvise or mlock error. abandon lock memory");
      //return false;
    }

  // Put the current cursor_ at the first frame of opened Page
  cursor_ = ((tPageHead*)buf_)->buf;
  return true;
}

