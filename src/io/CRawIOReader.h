/*
 * CRawIOReader.h
 *
 *  Created on: 2018年4月24日
 *      Author: hongxu
 */

#ifndef SRC_IO_CRAWIOREADER_H_
#define SRC_IO_CRAWIOREADER_H_

#include "CIOBase.h"

class CRawIOReader: public CIOBase
{
public:
	CRawIOReader();
	virtual ~CRawIOReader();

  // default open and move the latest frame in the latest page
	bool init(string path, long from_nano=-1);

	// from_page = -1 for the latest page, from_nano = -1 for the latest frame
	bool init(string path, int from_page, long from_nano);

	// -1 for tail
	void seek(int frame_no);

	void seekTime(long nano);

	void setReadPos(long nano);

	inline const char* read(uint32_t &len);

	inline void passFrame();

	inline const tFrameHead* getCurFrame();

	bool passPage();

	bool load(int num);

protected:
	char		*cursor_;
	int 		page_num_;
};

// Read the next frame, returning the pointer to the data bytes
// and 'len' of the data bytes
inline const char* CRawIOReader::read(uint32_t& len)
{
	if(fd_ >= 0)
	{
		uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
		if(cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail)
		{
			if(unlikely(!checkFrameValid(cursor_)))
			{
				LOG_ERR("frame format err, skip to the tail!");
				cursor_ = buf_ + ((volatile tPageHead* volatile)buf_)->tail;
				return NULL;
			}
			len = ((tFrameHead*)cursor_)->len;
			char *data = ((tFrameHead*)cursor_)->buf;
			cursor_ = data + len;
			return data;
		}
		else if(unlikely(status == PAGE_STATUS_FINISH))
		{
			unload();
			if(load(++page_num_))
			{
				return read(len);
			}
		}
	}
	else
	{
		if(load(page_num_))
		{
			return read(len);
		}
	}
	return NULL;
}

// Skip to the next frame
inline void CRawIOReader::passFrame()
{
	if(fd_ >= 0)
	{
		uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
		if(cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail)
		{
			cursor_ += ((tFrameHead*)cursor_)->len + sizeof(tFrameHead);
		}
		else if(unlikely(status == PAGE_STATUS_FINISH))
		{
			unload();
			load(++page_num_);
		}
	}
	else
	{
		load(page_num_);
	}
}

// Get the current frame
inline const tFrameHead* CRawIOReader::getCurFrame()
{
	if(fd_ >= 0)
	{
		uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
		if(cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail)
		{
			if(unlikely(!checkFrameValid(cursor_)))
			{
				LOG_ERR("frame format err, skip to the tail!");
				cursor_ = buf_ + ((volatile tPageHead* volatile)buf_)->tail;
				return NULL;
			}
			return (const tFrameHead*)cursor_;
		}
		else if(unlikely(status == PAGE_STATUS_FINISH))
		{
			unload();
			if(load(++page_num_))
			{
				return getCurFrame();
			}
		}
	}
	else
	{
		if(load(page_num_))
		{
			return getCurFrame();
		}
	}
	return NULL;
}

// Get the frame header based on the frame's data bytes pointer
inline tFrameHead* getIOFrameHead(const void *buf) {return (tFrameHead*)((const char*)buf - sizeof(tFrameHead));}

#endif /* SRC_IO_CRAWIOREADER_H_ */

