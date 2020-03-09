/*
 * CBuffer.cpp
 *
 *  Created on: 2018年4月13日
 *      Author: sky
 */

#include <algorithm>
#include "CBuffer.h"
using namespace std;

CBuffer::CBuffer(uint32_t buf_size) : p_(NULL), size_(buf_size), head_(0), tail_(0)
{
	p_ = (char*)malloc(buf_size);
}

CBuffer::~CBuffer()
{
	if(p_)
	{
		free(p_);
		p_ = NULL;
	}
}

bool CBuffer::cycleWrite(const char* p, uint32_t len)
{
	uint32_t l = left();
	if(l > len)
	{
		uint32_t m = tail_ % size_;
		uint32_t ml = min(size_ - m, len);
		memcpy(p_ + m, p, ml);
		memcpy(p_, p + ml, len - ml);
		tail_ += len;
		return true;
	}
	return false;
}

const char* CBuffer::cycleRead(uint32_t& length)
{
	uint32_t l = len();
	length = min(l, size_ - head_ % size_);
	return p_ + head_ % size_;
}

void CBuffer::getLeftVec(struct iovec v[2])
{
	memset(v, 0, sizeof(iovec) * 2);
	uint32_t m = tail_ % size_;
	v[0].iov_base = p_ + m;
	if(len() < m)
	{
		v[0].iov_len = size_ - m;
		v[1].iov_base = p_;
		v[1].iov_len = m + head_ - tail_;
	}
	else v[0].iov_len = left();
}

void CBuffer::getDataVec(struct iovec v[2])
{
	memset(v, 0, sizeof(iovec) * 2);
	uint32_t m = head_ % size_;
	v[0].iov_base = p_ + m;
	if(len() > size_ - m)
	{
		v[0].iov_len = size_ - m;
		v[1].iov_base = p_;
		v[1].iov_len = len() + m - size_;
	}
	else v[0].iov_len = len();
}
