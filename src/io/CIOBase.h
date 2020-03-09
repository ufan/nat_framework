/*
 * CIOBase.h
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#ifndef SRC_IO_CIOBASE_H_
#define SRC_IO_CIOBASE_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "ioprotocol.h"
#include "Logger.h"
#include "CTimer.h"
#include "compiler.h"
using namespace std;

#define IO_FILE_SUFFIX 	"_io_"

class CIOBase
{
public:
	CIOBase(uint32_t page_size=IO_PAGE_SIZE);
	virtual ~CIOBase();

	// set the file path prefix
	void setPrefix(string prefix) { prefix_ = prefix + IO_FILE_SUFFIX; }

	void setPageSize(uint32_t page_size) {size_ = page_size;}

	string getPrefix() {return prefix_;}

	void setMemMode(bool lock) { is_lock_ = lock;}

	int getTailNum();

	int getHeadNum();

	void getPageNum(vector<int> &vno);

	// -1 for tail page
	int getUpperPageNo(int page_no);

	bool load(int num, bool is_write=false);

	void unload();

	bool checkFrameValid(void *frame) { return FRAME_HEAD_MAGIC == ((tFrameHead*)frame)->magic; }

	bool checkPageHead(void *head)
	{
		return ((tPageHead*)head)->version == PAGE_HEAD_VER_1
				&& ((tPageHead*)head)->size == size_;
	}

	bool hasLoad() {return fd_ >= 0;}

	char* getRawBuffer(uint32_t size) {size = size_; return buf_;}

protected:
	char 		*buf_;
	uint32_t	size_;
	int			fd_;
	bool		is_lock_;
	string 		prefix_;
};

#endif /* SRC_IO_CIOBASE_H_ */
