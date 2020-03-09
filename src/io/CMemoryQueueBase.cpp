/*
 * CMemoryQueueBase.cpp
 *
 *  Created on: Sep 6, 2017
 *      Author: hongxu
 */

#include "CMemoryQueueBase.h"

#include <iostream>
#include <boost/functional/hash.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "compiler.h"
#include "Logger.h"

using namespace std;
using boost::hash;

namespace IPC {

CMemoryQueueBase::CMemoryQueueBase() :
		pstart_(NULL),
		lock_(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP),
		ifd_(-1)
{

}

CMemoryQueueBase* CMemoryQueueBase::instance()
{
	static CMemoryQueueBase s_instance;
	return &s_instance;
}

bool CMemoryQueueBase::init(string filename)
{
	if(pstart_)
	{
		return true;
	}

	bool isnew = false;
	if(access(filename.c_str(), F_OK) < 0)
	{
		isnew = true;
	}

	ifd_ = open(filename.c_str(), O_CREAT|O_RDWR, 0666);
	if(ifd_ < 0)
	{
		LOG_ERR("open file %s fail, err: %d", filename.c_str(), errno);
		return false;
	}

	struct stat statbuff;
	if(fstat(ifd_, &statbuff) < 0)
	{
		close(ifd_);
		ifd_ = -1;
		LOG_ERR("stat file %s fail, err: %d", filename.c_str(), errno);
		return false;
	}

	if(statbuff.st_size < sizeof(stQueueBase))
	{
		if(ftruncate(ifd_, sizeof(stQueueBase)) < 0)
		{
			close(ifd_);
			ifd_ = -1;
			LOG_ERR("truncate file %s fail, err: %d", filename.c_str(), errno);
			return false;
		}
	}

	void *pmap = mmap(0, sizeof(stQueueBase), PROT_READ|PROT_WRITE, MAP_SHARED, ifd_, 0);
	if(pmap == MAP_FAILED)
	{
		close(ifd_);
		LOG_ERR("mmap file failed: %d", errno);
		return false;
	}

	pstart_ = (stQueueBase*)pmap;
	if(isnew)
	{
		pstart_->queuecnt_ = 0;
	}

	return true;
}

bool CMemoryQueueBase::setFileLock(int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_WRLCK;
	lk.l_pid = getpid();

	if(fcntl(ifd_, F_SETLKW, &lk) < 0)
	{
		return false;
	}
	return true;
}

bool CMemoryQueueBase::tryFileLock(int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_WRLCK;
	lk.l_pid = getpid();

	if(fcntl(ifd_, F_SETLK, &lk) < 0)
	{
		return false;
	}
	return true;
}

bool CMemoryQueueBase::unsetFileLock(int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_UNLCK;
	lk.l_pid = 0;

	if(fcntl(ifd_, F_SETLK, &lk) < 0)
	{
		return false;
	}
	return true;
}

bool CMemoryQueueBase::globalLock(int off)
{
	pthread_mutex_lock(&lock_);
	return setFileLock(off);
}

bool CMemoryQueueBase::globalUnlock(int off)
{
	pthread_mutex_unlock(&lock_);
	return unsetFileLock(off);
}

bool CMemoryQueueBase::tryLockKey(uint64_t key)
{
	int off = key % sizeof(stQueueBase);

	pthread_mutex_lock(&lock_);
	bool ret = tryFileLock(off);
	pthread_mutex_unlock(&lock_);

	return ret;
}

bool CMemoryQueueBase::unLockKey(uint64_t key)
{
	int off = key % sizeof(stQueueBase);

	pthread_mutex_lock(&lock_);
	bool ret = unsetFileLock(off);
	pthread_mutex_unlock(&lock_);

	return ret;
}

uint32_t CMemoryQueueBase::getShmKeyByName(string ipc_name)
{
	stQueueInfo *pqinfo = pstart_->queue_info;
	for(int i = 0; i < pstart_->queuecnt_; i++)
	{
		if(ipc_name == pqinfo[i].ipc_name_)
		{
			return pqinfo[i].shmkey_;
		}
	}
	return 0;
}

uint32_t CMemoryQueueBase::registerQueue(string name)
{
	if(name.size() >= MAX_NAME_SIZE)
	{
		LOG_ERR("name(%s) too long", name.c_str());
		return 0;
	}

	uint32_t hs = getShmKeyByName(name);
	if(hs > 0) return hs;					// already exist

	boost::hash<string> hashStr;
	hs = (uint32_t)(hashStr(name) & 0xffffffff);
	hs += MAGIC_SHMKEY;

	globalLock();

	if(pstart_->queuecnt_ >= MAX_QUEUE)
	{
		globalUnlock();
		LOG_ERR("queue count reach max size");
		return 0;
	}

	stQueueInfo &info = pstart_->queue_info[pstart_->queuecnt_];
	strcpy(info.ipc_name_, name.c_str());
	info.shmkey_ = hs;
	pstart_->queuecnt_++;

	globalUnlock();

	return hs;
}

void CMemoryQueueBase::deleteQueue(string name)
{
	stQueueInfo *pqinfo = pstart_->queue_info;
	for(int i = 0; i < pstart_->queuecnt_; i++)
	{
		if(name == pqinfo[i].ipc_name_)
		{
			globalLock();

			pstart_->queuecnt_--;
			memcpy(pqinfo + i, pstart_->queue_info + pstart_->queuecnt_, sizeof(stQueueInfo));

			globalUnlock();
		}
	}
}

void CMemoryQueueBase::list()
{
	stQueueInfo *pqinfo = pstart_->queue_info;
	for(int i = 0; i < pstart_->queuecnt_; i++)
	{
		printf("key:0x%08x\tname:%s\n", pqinfo[i].shmkey_, pqinfo[i].ipc_name_);
	}

}

void CMemoryQueueBase::list(vector<string> &vques)
{
	stQueueInfo *pqinfo = pstart_->queue_info;
	for(int i = 0; i < pstart_->queuecnt_; i++)
	{
		vques.push_back(pqinfo[i].ipc_name_);
	}
}


} /* namespace IPC */

