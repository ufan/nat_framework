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

/*
 * CGlobalSafeRawIOWriter aims to be used as inter-process communication channel.
 * Multiple processes are allowed to fill in the Pages managed by it.
 * The race condition for the same Page managed by multiple process is solved by CGlobalLock.
 * Whenever a frame need to be filled in, the owning process will lock the Page exclusively
 * until filling finishing.
 * The class is not thread-safe, thus should not be used in multi-thread environment.
 * Use CSafeRawIOWriter in MT envrironment instead.
 */

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
