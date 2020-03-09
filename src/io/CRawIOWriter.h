/*
 * CRawIOWriter.h
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#ifndef SRC_IO_CRAWIOWRITER_H_
#define SRC_IO_CRAWIOWRITER_H_

#include <string>
#include "CIOBase.h"
using namespace std;


class CRawIOWriter : public CIOBase
{
public:
	CRawIOWriter(uint32_t page_size=IO_PAGE_SIZE);
	virtual ~CRawIOWriter();

	bool createNextPage();

	bool init(string path);

	bool write(const void* data, uint32_t len);

	char* prefetch(uint32_t len);
	void commit();

	bool load(int num);

protected:
	int 		page_num_;
	uint32_t	prefetch_tail_;
	uint32_t	page_size_;
	bool		is_test_lock_onload_;
};

class CSafeRawIOWriter : public CRawIOWriter
{
public:
	CSafeRawIOWriter(uint32_t page_size=IO_PAGE_SIZE) : CRawIOWriter(page_size)
	{
		is_test_lock_onload_ = false;
	}

	bool init(string path);
	bool write(const void* data, uint32_t len);
	char* prefetch(uint32_t len);
	void commit();
	void discard();
};

#endif /* SRC_IO_CRAWIOWRITER_H_ */
