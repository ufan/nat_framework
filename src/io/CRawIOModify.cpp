/*
 * CRawIOModify.cpp
 *
 *  Created on: 2018年11月2日
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
#include "CRawIOModify.h"

CRawIOModify::CRawIOModify()
{

}

CRawIOModify::~CRawIOModify()
{

}

bool CRawIOModify::loadFile(string path)
{
	fd_ = open(path.c_str(), O_RDWR, (mode_t)0666);
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

    buf_ = (char*)mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
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

