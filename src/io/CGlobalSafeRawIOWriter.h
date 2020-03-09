/*
 * CGlobalSafeRawIOWriter.h
 *
 *  Created on: 2018年5月9日
 *      Author: hongxu
 */

#ifndef SRC_IO_CGLOBALSAFERAWIOWRITER_H_
#define SRC_IO_CGLOBALSAFERAWIOWRITER_H_

#include "CRawIOWriter.h"
#include "CGlobalLock.h"

#define GLOBAL_SAFE_RAW_IO_LOCK_FILE "/tmp/at_safe_io.lock"

class CGlobalSafeRawIOWriter : public CRawIOWriter
{
public:
	CGlobalSafeRawIOWriter(uint32_t page_size=IO_PAGE_SIZE) : CRawIOWriter(page_size), hash_(0)
	{
		is_test_lock_onload_ = false;
	}

	bool init(string path);
	bool write(const void* data, uint32_t len);
	char* prefetch(uint32_t len);
	void commit();
	void discard();

private:
	uint32_t 			hash_;
	static CGlobalLock 	s_safe_io_lock_;
};


#endif /* SRC_IO_CGLOBALSAFERAWIOWRITER_H_ */
