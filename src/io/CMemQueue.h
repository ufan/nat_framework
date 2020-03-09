/*
 * CMemQueue.h
 *
 *  Created on: Sep 5, 2017
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CMEMQUEUE_H_
#define LIB_MEMORYQUEUE_CMEMQUEUE_H_

#include <sys/types.h>
#include <stdint.h>
#include <string>
#include <memory.h>
#include "compiler.h"
#include "Logger.h"

namespace IPC {

class CMemQueue
{
public:
	// this structure should be 8 bytes aligned
	struct stQueConfig
	{
		uint64_t	seq;
		int 		elemcnt;
		int 		elemsize;
	};

public:
	CMemQueue();
	virtual ~CMemQueue();

	bool init(std::string name, uint32_t elemcnt, uint32_t elemsize, bool write_lock=false);

	void destroy();

	void write(const void *obj)
	{
		uint32_t i = *pseq_ % elemcnt_;
		memcpy(pdata_ + aligned_elemsize_ * i, obj, elemsize_);
		(*pseq_)++;
	}

	int read(void *obj)
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

	void* read()
	{
		if(likely(read_seq_ < *pseq_))
		{
			uint32_t i = read_seq_ % elemcnt_;
			read_seq_++;
			return pdata_ + aligned_elemsize_ * i;
		}
		return NULL;
	}

	void resetReadSeq()
	{
		read_seq_ = *pseq_ / elemcnt_ * elemcnt_;		// simply from buffer head
	}

	void printInfo();

	void* preGetMem()
	{
		return pdata_ + (aligned_elemsize_ * (*pseq_ % elemcnt_));
	}

	void commit()
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
	int 					shmid_;
	uint32_t				locked_key_;
};


// maybe faster for object which is already 8 bytes aligned
template<class T>
class CObjQueue : public CMemQueue
{
public:
	CObjQueue() {}
	~CObjQueue(){}

	void write(const T &obj)
	{
		((T*)pdata_)[*pseq_ % elemcnt_] = obj;
		// asm volatile("sfence" ::: "memory");				// not need
		(*pseq_)++;
	}

	int read(T &obj)
	{
		if(likely(read_seq_ < *pseq_))
		{
			obj = ((T*)pdata_)[read_seq_ % elemcnt_];
			read_seq_++;
			return sizeof(T);
		}
		return 0;
	}

	T* read()
	{
		if(likely(read_seq_ < *pseq_))
		{
			int i = read_seq_ % elemcnt_;
			read_seq_++;
			return ((T*)pdata_) + i;
		}
		return NULL;
	}

	T* blockread()
	{
		while(read_seq_ == *pseq_) ; //asm volatile("pause");

		int i = read_seq_ % elemcnt_;
		read_seq_++;
		return ((T*)pdata_) + i;
	}

	T* preGetMem()
	{
		return ((T*)pdata_) + (*pseq_ % elemcnt_);
	}

	bool init(std::string name, uint32_t elemcnt, bool write_lock=false)
	{
		return CMemQueue::init(name, elemcnt, sizeof(T), write_lock);
	}
};


} /* namespace IPC */

#endif /* LIB_MEMORYQUEUE_CMEMQUEUE_H_ */
