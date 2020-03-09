/*
 * CBuffer.h
 *
 *  Created on: 2018年4月13日
 *      Author: sky
 */

#ifndef MAPPORT_CBUFFER_H_
#define MAPPORT_CBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

class CBuffer
{
public:
	struct tVec
	{
		const char 	*p;
		uint32_t 	len;
	};

public:
	CBuffer(uint32_t buf_size_=4096);
	virtual ~CBuffer();

	void resize(uint32_t size)
	{
		void *p = realloc(p_, size);
		if(p)  // if p is null, then nothing changed...
		{
			p_ = (char*)p;
			size_ = size;
		}
	}

	uint32_t len() {return tail_ - head_;}

	uint32_t left() {return size_ - len();}

	bool empty() {return head_ == tail_;}

	bool full() {return len() == size_;}

	void clear() {head_ = tail_ = 0;}

	bool cycleWrite(const char *p, uint32_t len);

	const char* cycleRead(uint32_t &len);

	void commitRead(uint32_t len) {head_ += len;}

	void commitWrite(uint32_t len) {tail_ += len;}

	void getLeftVec(struct iovec v[2]);

	void getDataVec(struct iovec v[2]);

public:
	char			*p_;
	uint32_t		size_;
	uint32_t 		head_;
	uint32_t		tail_;
};

#endif /* MAPPORT_CBUFFER_H_ */
