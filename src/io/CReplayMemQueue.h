/*
 * CReplayMemQueue.h
 *
 *  Created on: 2017年11月8日
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CREPLAYMEMQUEUE_H_
#define LIB_MEMORYQUEUE_CREPLAYMEMQUEUE_H_

#include "CLocalQueue.h"
#include <map>
#include "ReplayManager.h"
using namespace std;

template<class T>
class CReplayMemQueue : public CLocalQueue
{
public:
	CReplayMemQueue(void *p_owner, void (*finish_func)(void*)):
		p_owner_(p_owner), finish_func_(finish_func)
	{

	}
	virtual ~CReplayMemQueue() {}

	bool init(std::string name, uint32_t elemcnt, bool write_lock=false)
	{
		return true;
	}

	void* read()
	{
		T* obj = (T*)ReplayManager::ReadMd();
		if(!obj)	// no more data
		{
			if(finish_func_) finish_func_(p_owner_);
		}
		return obj;
	}

protected:
	void			*p_owner_;
	void    		(*finish_func_)(void*);
};

#endif /* LIB_MEMORYQUEUE_CREPLAYMEMQUEUE_H_ */
