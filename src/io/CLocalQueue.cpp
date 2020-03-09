/*
 * CLocalQueue.cpp
 *
 *  Created on: 2017年11月8日
 *      Author: hongxu
 */

#include "CLocalQueue.h"
#include <errno.h>

map<string, CLocalQueue::tQueueInfo> CLocalQueue::s_queumap_;
pthread_mutex_t	CLocalQueue::s_globallock_ = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

CLocalQueue::CLocalQueue() : islocked_(false)
{

}

CLocalQueue::~CLocalQueue()
{
	map<string, tQueueInfo>::iterator iter = s_queumap_.find(name_);
	if(iter != s_queumap_.end())
	{
		if(islocked_)
		{
			pthread_mutex_unlock(&iter->second.lock);
			islocked_ = false;
		}
	}
}

bool CLocalQueue::init(std::string name, uint32_t elemcnt, uint32_t elemsize, bool write_lock)
{
	if(s_queumap_.find(name) == s_queumap_.end())
	{
		pthread_mutex_lock(&s_globallock_);
		tQueueInfo & info = s_queumap_[name];
		info.elemcnt = elemcnt;
		info.elemsize = elemsize;
		info.lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
		int aligned_elemsize = elemsize & 7 ? elemsize + 8 - (elemsize & 7) : elemsize;
		info.pdata = new int8_t[aligned_elemsize * elemcnt];
		info.seq = 0;
		pthread_mutex_unlock(&s_globallock_);
	}

	tQueueInfo & info = s_queumap_[name];
	if(write_lock)
	{
		if(0 != pthread_mutex_trylock(&info.lock))
		{
			LOG_ERR("lock queue %s err:%d", name.c_str(), errno);
			return false;
		}
	}
	islocked_ = write_lock;

	pseq_ = &(info.seq);
	pdata_		= info.pdata;
	elemcnt_ 	= info.elemcnt;
	elemsize_ 	= info.elemsize;
	aligned_elemsize_ = elemsize_ & 7 ? elemsize_ + 8 - (elemsize_ & 7) : elemsize_;
	read_seq_	= info.seq;						// read from tail, the newest data

	name_ = name;
	return true;
}

void CLocalQueue::destroy()
{
	pthread_mutex_lock(&s_globallock_);
	map<string, tQueueInfo>::iterator iter = s_queumap_.find(name_);
	if(iter != s_queumap_.end())
	{
		if(islocked_)
		{
			pthread_mutex_unlock(&iter->second.lock);
			islocked_ = false;
		}
		s_queumap_.erase(iter);
	}
	pthread_mutex_unlock(&s_globallock_);
}

