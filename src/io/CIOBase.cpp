/*
 * CIOBase.cpp
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
#include "CIOBase.h"
#include "utils.h"


CIOBase::CIOBase(uint32_t page_size) : buf_(NULL), size_(page_size), fd_(-1), is_lock_(true)
{

}

CIOBase::~CIOBase()
{
	unload();
}

int CIOBase::getTailNum()
{
	int num = -1;
    glob_t buf;
    int i;
    string pre = prefix_ + "[0-9]*";
    glob(pre.c_str(), GLOB_NOSORT, NULL, &buf);

    for(i = 0; i < buf.gl_pathc; i++)
    {
        int n = atoi(buf.gl_pathv[i] + prefix_.size());
        num = n > num ? n : num;
    }

   globfree(&buf);
   return num;
}

int CIOBase::getHeadNum()
{
	int num = INT_MAX;
    glob_t buf;
    int i;
    string pre = prefix_ + "[0-9]*";
    glob(pre.c_str(), GLOB_NOSORT, NULL, &buf);

    for(i = 0; i < buf.gl_pathc; i++)
    {
    	int n = atoi(buf.gl_pathv[i] + prefix_.size());
        num = n < num ? n : num;
    }

   globfree(&buf);
   return num < INT_MAX ? num : -1;
}

void CIOBase::getPageNum(vector<int> &vno)
{
    glob_t buf;
    string pre = prefix_ + "[0-9]*";
    glob(pre.c_str(), GLOB_NOSORT, NULL, &buf);

    for(int i = 0; i < buf.gl_pathc; i++)
    {
    	int n = atoi(buf.gl_pathv[i] + prefix_.size());
    	vno.push_back(n);
    }
    globfree(&buf);

    sort(vno.begin(), vno.end());
}

int CIOBase::getUpperPageNo(int page_no)
{
	if(page_no >= 0)
	{
		vector<int> vnos;
		getPageNum(vnos);
		auto itr = lower_bound(vnos.begin(), vnos.end(), page_no);
		return itr != vnos.end() ? *itr : -1;
	}
	else
	{
		return getTailNum();
	}
	return -1;
}

bool CIOBase::load(int num, bool is_write)
{
	string path = prefix_ + to_string(num);

	fd_ = open(path.c_str(), (is_write) ? (O_RDWR | O_CREAT) : O_RDONLY, (mode_t)0666);
    if (fd_ < 0)
    {
        LOG_ERR("Cannot open file %s, err:%s", path.c_str(), strerror(errno));
        return false;
    }

	struct stat statbuff;
	if(fstat(fd_, &statbuff) < 0)
	{
		close(fd_); fd_ = -1;
		LOG_ERR("stat file %s fail, err:%s", path.c_str(), strerror(errno));
		return false;
	}

	if(is_write)
	{
    	if(!tryFileLock(fd_, 0))
    	{
    		LOG_ERR("file %s already has a writer.", path.c_str());
    		close(fd_); fd_ = -1;
    		return false;
    	}

		if(ftruncate(fd_, size_) < 0)
		{
			close(fd_); fd_ = -1;
			LOG_ERR("truncate file %s fail, err:%s", path.c_str(), strerror(errno));
			return false;
		}
	}
	else size_ = statbuff.st_size;

    buf_ = (char*)mmap(0, size_, (is_write) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd_, 0);
    if ((void*)buf_ == MAP_FAILED)
    {
    	close(fd_); fd_ = -1;
        LOG_ERR(" mapping file to buffer err:%s", strerror(errno));
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

void CIOBase::unload()
{
	if(fd_ >= 0)
	{
	    //unlock and unmap
	    if (is_lock_ && munlock(buf_, size_) != 0)
	    {
	        LOG_ERR("munlock err:%s", strerror(errno));
	    }

	    if(munmap(buf_, size_)!=0)
	    {
	        LOG_ERR("munmap err:%s", strerror(errno));
	    }

	    close(fd_);
	    fd_ = -1;
	}
}
