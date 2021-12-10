/*
 * CRawIOReader.h
 *
 *  Created on: 2018年4月24日
 *      Author: hongxu
 *
 * Comment by Yong:
 * IOReader will not have r/w conflicts with IOWriter in multi-thread,
 * multi-process environment. The methods for seeking the read position using
 * timestamp or frame number are based on the page head information: last_nano
 * and tail. The two fields records the timestamp and the frame tail of the last
 * filled-in frame. They are updated only after the frame body has been written,
 * thus ensuring that the frame body is always complete corresponding the
 * timestamp and the tail at any moment.
 */

#ifndef SRC_IO_CRAWIOREADER_H_
#define SRC_IO_CRAWIOREADER_H_

#include "CIOBase.h"

class CRawIOReader : public CIOBase {
 public:
  CRawIOReader();
  virtual ~CRawIOReader();

  // default open and move the latest frame in the latest page
  bool init(string path, long from_nano = -1);

  // from_page = -1 for the latest page, from_nano = -1 for the latest frame
  bool init(string path, int from_page, long from_nano);

  // -1 for tail
  void seek(int frame_no);

  void seekTime(long nano);

  void setReadPos(long nano);

  inline const char* read(uint32_t& len);

  inline void passFrame();

  inline const tFrameHead* getCurFrame();

  // Load the next page if current one is full or no page is opened currently.
  // Return false and print error, if no new page is available for reading.
  // If the current page is not full, the method will do nothing and just return
  // false.
  bool passPage();

  bool load(int num);

 protected:
  char* cursor_;  // pointer to the frame for reading
  int page_num_;  // index number of the current page file opened for reading
};

// Read the current frame pointed by cursor_, returning the pointer to the data
// bytes and 'len' of the data bytes. cursor_ is also updated to the next frame
// in this group of pages.
// Return null if no new frame available.
inline const char* CRawIOReader::read(uint32_t& len) {
  if (fd_ >= 0) {
    uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
    if (cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail) {
      if (unlikely(!checkFrameValid(cursor_))) {
        LOG_ERR("frame format err, skip to the tail!");
        cursor_ = buf_ + ((volatile tPageHead* volatile)buf_)->tail;
        return NULL;
      }
      len = ((tFrameHead*)cursor_)->len;
      char* data = ((tFrameHead*)cursor_)->buf;
      cursor_ = data + len;
      return data;
    } else if (unlikely(status == PAGE_STATUS_FINISH)) {
      unload();
      if (load(++page_num_)) {
        return read(len);
      }
    }
  } else {
    if (load(page_num_)) {
      return read(len);
    }
  }
  return NULL;
}

// Skip to the next frame, i.e. update cursor to the beginning of the next frame
// in the same page. If current cursor points to the end of a full page, next
// page will be loaded and the cursor will point to the first frame of the new
// page.
inline void CRawIOReader::passFrame() {
  if (fd_ >= 0) {
    uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
    if (cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail) {
      cursor_ += ((tFrameHead*)cursor_)->len + sizeof(tFrameHead);
    } else if (unlikely(status == PAGE_STATUS_FINISH)) {
      unload();
      load(++page_num_);
    }
  } else {
    load(page_num_);
  }
}

// Get the current readable frame. Load new page if current page is full,
// and the current readable frame is the first frame of the new page.
// Return null if no readable frame available.
inline const tFrameHead* CRawIOReader::getCurFrame() {
  if (fd_ >= 0) {
    uint8_t status = ((volatile tPageHead* volatile)buf_)->status;
    if (cursor_ < buf_ + ((volatile tPageHead* volatile)buf_)->tail) {
      if (unlikely(!checkFrameValid(cursor_))) {
        LOG_ERR("frame format err, skip to the tail!");
        cursor_ = buf_ + ((volatile tPageHead* volatile)buf_)->tail;
        return NULL;
      }
      return (const tFrameHead*)cursor_;
    } else if (unlikely(status == PAGE_STATUS_FINISH)) {
      unload();
      if (load(++page_num_)) {
        return getCurFrame();
      }
    }
  } else {
    if (load(page_num_)) {
      return getCurFrame();
    }
  }
  return NULL;
}

// Get the frame header based on the frame's data bytes pointer
inline tFrameHead* getIOFrameHead(const void* buf) {
  return (tFrameHead*)((const char*)buf - sizeof(tFrameHead));
}

#endif /* SRC_IO_CRAWIOREADER_H_ */
