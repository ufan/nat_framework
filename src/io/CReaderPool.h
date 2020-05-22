/*
 * CReaderPool.h
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#ifndef SRC_IO_CREADERPOOL_H_
#define SRC_IO_CREADERPOOL_H_

#include <vector>
#include <memory>
#include <string>
#include <stdint.h>
#include "CRawIOReader.h"
using namespace std;

/*
 * CReaderPool is a collection of CRawIOReader, using hash id as key
 */

class CReaderPool
{
	typedef unique_ptr<CRawIOReader> CRawIOReaderPtr;

	class CReaderInfo
	{
	public:
		CReaderInfo(CRawIOReaderPtr p, uint32_t h) : reader_(move(p)), hash_(h) {}
		CRawIOReaderPtr		reader_;
		uint32_t			hash_;
	};

public:
	CReaderPool() {}
	virtual ~CReaderPool() {}

  // Checking the existence of a reader with 'hash'
	bool hasHash(uint32_t hash)
	{
		for(auto itr = readers_.begin(); itr != readers_.end(); ++itr)
		{
			if(itr->hash_ == hash)
			{
				return true;
			}
		}
		return false;
	}

  // Adding a reader with id 'hash' using 'path', 'from_page' and 'from_nano' as filter
	bool add(uint32_t hash, string path, int from_page, long from_nano)
	{
		if(hasHash(hash)) return false;
		CRawIOReaderPtr p_reader(new CRawIOReader);
		bool ret = p_reader->init(path, from_page, from_nano);
		if(ret) readers_.emplace_back(move(p_reader), hash);
		return ret;
	}

	void add(uint32_t hash, CRawIOReader *p)
	{
		if(hasHash(hash)) return;
		readers_.emplace_back(CRawIOReaderPtr(p), hash);
	}

  // Read the first available frame in all readers in the pool
	const char* read(uint32_t &len, uint32_t &hash)
	{
		for(auto &i : readers_)
		{
			const char* p = i.reader_->read(len);
			if(p)
			{
				hash = i.hash_;
				return p;
			}
		}
		return NULL;
	}

  // Read the earliest frame among all readers
	const char* seqRead(uint32_t &len, uint32_t &hash)
	{
		CReaderInfo *cur_reader = NULL;
		long min_nano = -1;
		for(auto &i : readers_)
		{
			const tFrameHead* p = i.reader_->getCurFrame();
			if(p && (min_nano < 0 || min_nano > p->nano))
			{
				min_nano = p->nano;
				cur_reader = &i;
			}
		}
		if(cur_reader)
		{
			const char* p = cur_reader->reader_->read(len);
			hash = cur_reader->hash_;
			return p;
		}
		return NULL;
	}

  // Move out a reader
	void erase(uint32_t hash)
	{
		for(auto itr = readers_.begin(); itr != readers_.end(); ++itr)
		{
			if(itr->hash_ == hash)
			{
				readers_.erase(itr);
				break;
			}
		}
	}

	int size() {return readers_.size();}

private:
	vector<CReaderInfo>		readers_;
};

#endif /* SRC_IO_CREADERPOOL_H_ */
