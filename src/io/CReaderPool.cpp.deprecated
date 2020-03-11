/*
 * CReaderPool.cpp
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#include "CReaderPool.h"

CReaderPool::CReaderPool()
{

}

CReaderPool::~CReaderPool()
{

}

bool CReaderPool::add(string path, int from_page, int from_nano)
{
	CRawIOReaderPtr p_reader(new CRawIOReader);
	bool ret = p_reader->init(path, from_page, from_nano);
	if(ret) readers_.emplace_back(move(p_reader));
	return ret;
}

const char* CReaderPool::read(uint32_t& len)
{
	const char* p = NULL;
	for(auto &i : readers_)
	{
		if(i.reader_->hasLoad())
		{
			p = i.reader_->read(len);
			if(p) break;
		}
		else
		{
			if(++i.cur_cnt_ > i.wait_cnt_)
			{
				i.cur_cnt_ = 0;
				p = i.reader_->read(len);
				if(p)
				{
					i.wait_cnt_ >>= 1;		// div 2
					break;
				}
				else
				{
					i.wait_cnt_ = i.wait_cnt_ < MAX_WAIT_CNT ? i.wait_cnt_ + 4 : MAX_WAIT_CNT;
				}
			}
		}
	}
	return p;
}

