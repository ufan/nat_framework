/*
 * CGlobalLock.cpp
 *
 *  Created on: 2018年4月25日
 *      Author: hongxu
 */

#include <stdio.h>
#include "CGlobalLock.h"
#include "utils.h"


CGlobalLock::CGlobalLock() : fd_(-1), file_size_(0), flag_(ATOMIC_FLAG_INIT)
{

}

bool CGlobalLock::init(const char *lockfile, int lock_file_size)
{
	if(access(lockfile, F_OK) < 0)
	{
		fd_ = open(lockfile, O_CREAT|O_RDWR, (mode_t)0666);
		if(fd_ >= 0)
		{
			if(ftruncate(fd_, lock_file_size) < 0)
			{
				perror("ftruncate lock file err:");
				close(fd_); fd_ = -1;
				return false;
			}
		}
		else
		{
			perror("create lock file err:");
			return false;
		}
		file_size_ = lock_file_size;
	}
	else
	{
		fd_ = open(lockfile, O_RDWR, (mode_t)0666);
		struct stat statbuff;
		if(fstat(fd_, &statbuff) < 0)
		{
			perror("stat lock file err:");
			close(fd_); fd_ = -1;
			return false;
		}
		file_size_ = statbuff.st_size;
	}
	return true;
}

CGlobalLock::~CGlobalLock()
{
	if(fd_ >= 0) close(fd_);
}

void CGlobalLock::lock(uint64_t hash)
{
	while (flag_.test_and_set(memory_order_acquire));
	setFileLock(fd_, hash % file_size_);
}

void CGlobalLock::unlock(uint64_t hash)
{
	unsetFileLock(fd_, hash % file_size_);
	flag_.clear(memory_order_release);
}

bool CGlobalLock::trylock(uint64_t hash)
{
	while (flag_.test_and_set(memory_order_acquire));
	bool ret = tryFileLock(fd_, hash % file_size_);
	flag_.clear(memory_order_release);

	return ret;
}

