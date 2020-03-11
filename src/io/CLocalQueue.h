/*
 * CLocalQueue.h
 *
 *  Created on: 2017年11月8日
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CLOCALQUEUE_H_
#define LIB_MEMORYQUEUE_CLOCALQUEUE_H_

#include "CQueueBase.h"
#include <map>
#include <pthread.h>
#include "SimpleLog.h"
using namespace std;

class CLocalQueue : public CQueueBase
{
public:
	struct tQueueInfo
	{
		uint64_t   		seq;
		int8_t*	   		pdata;
		int 			elemcnt;
		int 			elemsize;
		pthread_mutex_t	lock;
	};

public:
	CLocalQueue();
	virtual ~CLocalQueue();

	virtual bool init(std::string name, uint32_t elemcnt, uint32_t elemsize, bool write_lock=false);

	virtual void destroy();

protected:
	bool 		islocked_;
	string 		name_;

	static map<string, tQueueInfo>		s_queumap_;
	static pthread_mutex_t				s_globallock_;
};




#endif /* LIB_MEMORYQUEUE_CLOCALQUEUE_H_ */
