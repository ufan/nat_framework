/*
 * COrderMmap.cpp
 *
 *  Created on: 2018年6月3日
 *      Author: sky
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
#include "COrderTrackMmap.h"
#include "SysConf.h"
#include "Logger.h"
#include "utils.h"

COrderTrackMmap::COrderTrackMmap(bool lockmem) : is_lock_(lockmem)
{

}

COrderTrackMmap::~COrderTrackMmap()
{
	unload();
}

// Create the order track memory-mapped file
bool COrderTrackMmap::load(string name, bool is_write)
{
  // Create the directory where the order tracks will be saved
	if(!createPath(ORDER_TRACK_MMAP_PATH))
	{
		LOG_ERR("create dir" ORDER_TRACK_MMAP_PATH "err");
		return false;
	}

  // Create the order track file with engine's name as file name
	string path = string(ORDER_TRACK_MMAP_PATH) + name + ".ot";
	fd_ = open(path.c_str(), (is_write) ? (O_RDWR | O_CREAT) : O_RDONLY, (mode_t)0666);
  if (fd_ < 0)
  {
    LOG_ERR("Cannot open file %s, err:%s", path.c_str(), strerror(errno));
    return false;
  }

  // Get the status struct of the newly-opened file
	struct stat statbuff;
	if(fstat(fd_, &statbuff) < 0)
	{
		close(fd_); fd_ = -1;
		LOG_ERR("stat file %s fail, err:%s", path.c_str(), strerror(errno));
		return false;
	}

  // The size of the mmap file is the same as tOrderTrackMmap instance
	const uint32_t size = sizeof(tOrderTrackMmap);
	if(is_write)
	{
    // Try to lock the mmap file for writing
    // It has the effect of limiting the mmap file writable only to current process if success
    if(!tryFileLock(fd_, 0))
    {
      LOG_ERR("file %s already has a writer.", path.c_str());
      close(fd_); fd_ = -1;
      return false;
    }

    // Make sure the required size of space if available
		if(ftruncate(fd_, size) < 0)
		{
			close(fd_); fd_ = -1;
			LOG_ERR("truncate file %s fail, err:%s", path.c_str(), strerror(errno));
			return false;
		}
	}
  // If opened for reading, make sure the size match
	else if(statbuff.st_size < size)
	{
		close(fd_); fd_ = -1;
		LOG_ERR("order track mmap size mismatch.");
		return false;
	}

  // Create memory-mapped file into buffer, this buffer is shared between process
  buf_ = (tOrderTrackMmap*)mmap(0, size, (is_write) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd_, 0);
  if ((void*)buf_ == MAP_FAILED)
  {
    close(fd_); fd_ = -1;
    LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
    return false;
  }

  // try to ask kernel to allocate memory effieciently
	if (is_lock_ && (madvise(buf_, size, MADV_SEQUENTIAL) != 0 || mlock(buf_, size) != 0))
	{
    //munmap(buf_, size);
    //close(fd_); fd_ = -1;
    LOG_WARN("madvise or mlock error. abandon lock memory");
    //return false;
	}
  return true;
}

// Unlock and unmap
void COrderTrackMmap::unload()
{
	if(fd_ >= 0)
	{
	    if (is_lock_ && munlock(buf_, sizeof(tOrderTrackMmap)) != 0)
	    {
	        LOG_ERR("munlock err:%s", strerror(errno));
	    }

	    if(munmap(buf_, sizeof(tOrderTrackMmap))!=0)
	    {
	        LOG_ERR("munmap err:%s", strerror(errno));
	    }

	    close(fd_);
	    fd_ = -1;
	}
}

// Clear the current tracked orders, rest to empty
void COrderTrackMmap::clearTrack()
{
	if(fd_ >= 0 && buf_)
	{
		memset(buf_->order_track, 0, sizeof(buf_->order_track));
	}
}

