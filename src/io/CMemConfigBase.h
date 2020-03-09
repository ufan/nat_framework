/*
 * CMemConfigBase.h
 *
 *  Created on: Sep 11, 2017
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CMEMCONFIGBASE_H_
#define LIB_MEMORYQUEUE_CMEMCONFIGBASE_H_

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "Compiler.h"
#include "Log.h"


namespace IPC {

template<class T, uint32_t MAXCNT>
struct tMemConfig
{
	uint32_t			reserve_;
	int 				nodecnt_;
	T					nodes_[MAXCNT];
};

template<class T>
class CMemConfigBase {
public:
	CMemConfigBase():
		pstart_(NULL),
		lock_(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP),
		ifd_(-1)
	{

	}

	virtual ~CMemConfigBase()
	{
		if(pstart_)
		{
			munmap(pstart_, sizeof(T));
			pstart_ = NULL;

			close(ifd_);
			ifd_ = -1;
		}
	}

	bool init(std::string filename)
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

		if(statbuff.st_size < sizeof(T))
		{
			if(ftruncate(ifd_, sizeof(T)) < 0)
			{
				close(ifd_);
				ifd_ = -1;
				LOG_ERR("truncate file %s fail, err: %d", filename.c_str(), errno);
				return false;
			}
		}

		void *pmap = mmap(0, sizeof(T), PROT_READ|PROT_WRITE, MAP_SHARED, ifd_, 0);
		if(pmap == MAP_FAILED)
		{
			close(ifd_);
			LOG_ERR("mmap file failed: %d", errno);
			return false;
		}

		pstart_ = (T*)pmap;
		if(isnew)
		{
			pstart_->nodecnt_ = 0;
		}

		return true;
	}

	bool setFileLock(int off)
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

	bool unsetFileLock(int off)
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

	bool globalLock()
	{
		pthread_mutex_lock(&lock_);
		return setFileLock(0);
	}

	bool globalUnlock()
	{
		pthread_mutex_unlock(&lock_);
		return unsetFileLock(0);
	}

	T* getNode(const T &node)
	{
		T *n = pstart_->nodes_;
		for(int i = 0; i < pstart_->nodecnt_; i++)
		{
			if(node == n[i])
			{
				return n + i;
			}
		}
		return NULL;
	}

	bool registerNode(const T &node)
	{
		T *n = getNode(node);
		if(n)
		{
			*n = node;
			return true;
		}

		globalLock();

		if((char*)(pstart_->nodes_ + pstart_->nodecnt_) >= (char*)(pstart_ + 1))
		{
			globalUnlock();
			LOG_ERR("node count reach max size");
			return false;
		}

		T &target = pstart_->nodes_[pstart_->nodecnt_];
		target = node;
		pstart_->nodecnt_++;

		globalUnlock();

		return true;
	}

	void deleteNode(const T &node)
	{
		T *n = pstart_->nodes_;
		for(int i = 0; i < pstart_->nodecnt_; i++)
		{
			if(node == n[i])
			{
				globalLock();

				pstart_->nodecnt_--;
				n[i] = n[pstart_->nodecnt_];

				globalUnlock();
			}
		}
	}

	void list(std::vector<T> &v)
	{
		T *n = pstart_->nodes_;
		for(int i = 0; i < pstart_->nodecnt_; i++)
		{
			v.push_back(n[i]);
		}
	}

protected:
	T 						*pstart_;
	pthread_mutex_t			lock_;
	int						ifd_;
};

} /* namespace IPC */

#endif /* LIB_MEMORYQUEUE_CMEMCONFIGBASE_H_ */

