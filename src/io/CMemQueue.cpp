/*
 * CMemQueue.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: hongxu
 */

#include "CMemQueue.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <errno.h>
#include "Logger.h"
#include "compiler.h"
#include "CMemoryQueueBase.h"

using namespace std;

namespace IPC {

CMemQueue::CMemQueue():
		pdata_(NULL),
		pseq_(NULL),
		read_seq_(0),
		elemcnt_(0),
		aligned_elemsize_(0),
		elemsize_(0),
		shmid_(-1),
		locked_key_(0)
{

}

CMemQueue::~CMemQueue()
{
	if(pseq_)
	{
		shmdt((void*)pseq_);
	}

	if(locked_key_)
	{
		CMemoryQueueBase::instance()->unLockKey(locked_key_);
	}
}


bool CMemQueue::init(string name, uint32_t elemcnt, uint32_t elemsize, bool write_lock)
{
	if(pseq_)
	{
		LOG_ERR("fail: already init.");
		return false;
	}

	uint32_t shmkey = CMemoryQueueBase::instance()->registerQueue(name);
	if(shmkey == 0)
	{
		LOG_ERR("init queue %s err.", name.c_str());
		return false;
	}

	if(write_lock)
	{
		if(!CMemoryQueueBase::instance()->tryLockKey(shmkey))
		{
			LOG_ERR("lock queue %s err:%d", name.c_str(), errno);
			return false;
		}
		locked_key_ = shmkey;
	}

	bool isnew = false;
	int shmflag = SHM_R | SHM_W | SHM_NORESERVE;
	int shmid = shmget(shmkey, 0, shmflag);
	if(shmid == -1)
	{
		if(errno == ENOENT)		// not exist before
		{
			if(elemcnt == 0 || elemsize == 0)
			{
				LOG_ERR("element size or count can't be 0 for new queue.");
				return false;
			}

			aligned_elemsize_ = elemsize & 7 ? elemsize + 8 - (elemsize & 7) : elemsize;
			uint32_t totsize = sizeof(stQueConfig) + aligned_elemsize_ * elemcnt;
			shmflag |= IPC_CREAT;
			if(totsize >= (2 * 1024 * 1024)) shmflag |= SHM_HUGETLB;
			shmid = shmget(shmkey, totsize, shmflag);
			if(shmid == -1)
			{
				LOG_ERR("create shm err: %d", errno);
				return false;
			}
			if(shmctl(shmid, SHM_LOCK, NULL) < 0)		// don't swap
			{
				LOG_WARN("lock shm err: %d", errno);
			}
			isnew = true;
		}
		else
		{
			LOG_ERR("failed to get shared memory: %d", errno);
			return false;
		}
	}

	shmid_ = shmid;					// store this.
	void *pstart = shmat(shmid, NULL, 0);
	if(pstart == (void*)(-1))
	{
		LOG_ERR("shmat err:%d", errno);
		return false;
	}

	pseq_ = (uint64_t*)pstart;
	stQueConfig* pcfg = (stQueConfig*)pstart;
	if(isnew)
	{
		pcfg->elemcnt = elemcnt;
		pcfg->elemsize = elemsize;
		pcfg->seq = 0;
	}

	pdata_		= (int8_t*)pstart + sizeof(stQueConfig);
	elemcnt_ 	= pcfg->elemcnt;
	elemsize_ 	= pcfg->elemsize;
	aligned_elemsize_ = elemsize_ & 7 ? elemsize_ + 8 - (elemsize_ & 7) : elemsize_;
	read_seq_	= pcfg->seq;						// read from tail, the newest data

	uint32_t totsize = sizeof(stQueConfig) + aligned_elemsize_ * elemcnt_;
	if( 0 > madvise(pstart, totsize, MADV_WILLNEED))
	{
		LOG_WARN("madvise shm err: %d", errno);
	}

	return true;
}


void CMemQueue::destroy()
{
	if(pseq_)
	{
		shmdt((void*)pseq_);
		pseq_ = NULL;
	}

	if(shmid_ >= 0)
	{
		shmctl(shmid_, SHM_UNLOCK, NULL);
		shmctl(shmid_, IPC_RMID, NULL);		// delete this shm
		shmid_ = -1;
	}
}

void CMemQueue::printInfo()
{
	stQueConfig* pcfg = (stQueConfig*)pseq_;
	printf("seq:%llu elesize:%d elecnt:%d\n", (unsigned long long)pcfg->seq, pcfg->elemsize, pcfg->elemcnt);
}

} /* namespace IPC */

