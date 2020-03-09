/*
 * CQueueBase.h
 *
 *  Created on: 2017年11月8日
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CQUEUEBASE_H_
#define LIB_MEMORYQUEUE_CQUEUEBASE_H_

#include <sys/types.h>
#include <stdint.h>
#include <string>
#include <memory.h>
#include "compiler.h"
#include "Log.h"

class CQueueBase
{
public:
	CQueueBase():
		pseq_(NULL),
		pdata_(NULL),
		read_seq_(0),
		elemcnt_(0),
		aligned_elemsize_(0),
		elemsize_(0)
		{}

	virtual ~CQueueBase() {}

	virtual bool init(std::string name, uint32_t elemcnt, bool write_lock=false) {return false;}

	virtual void destroy() {}

	virtual void* write(const void *obj)
	{
		uint32_t i = *pseq_ % elemcnt_;
		void *p = pdata_ + aligned_elemsize_ * i;
		memcpy(p, obj, elemsize_);
		(*pseq_)++;
		return p;
	}

	virtual int read(void *obj)
	{
		if(likely(read_seq_ < *pseq_))
		{
			uint32_t i = read_seq_ % elemcnt_;
			memcpy(obj, pdata_ + aligned_elemsize_ * i, elemsize_);
			read_seq_++;
			return elemsize_;
		}
		return 0;
	}

	virtual void* read()
	{
		if(likely(read_seq_ < *pseq_))
		{
			uint32_t i = read_seq_ % elemcnt_;
			read_seq_++;
			return pdata_ + aligned_elemsize_ * i;
		}
		return NULL;
	}

	virtual void resetReadSeq()
	{
		read_seq_ = *pseq_ / elemcnt_ * elemcnt_;		// simply from buffer head
	}

	virtual void* preGetMem()
	{
		return pdata_ + (aligned_elemsize_ * (*pseq_ % elemcnt_));
	}

	virtual void commit()
	{
		(*pseq_)++;
	}

protected:
	volatile uint64_t		*pseq_;
	int8_t					*pdata_;
	uint64_t				read_seq_;
	int 					elemcnt_;
	int 					aligned_elemsize_;
	int 					elemsize_;
};

#endif /* LIB_MEMORYQUEUE_CQUEUEBASE_H_ */
