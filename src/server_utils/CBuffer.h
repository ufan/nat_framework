/*
 * CBuffer.h
 *
 *  Created on: 2018年4月13日
 *      Author: sky
 *
 * Comment by Yong:
 * CBuffer implements a ring buffer. If writing reaches end of buffer, it will
 * write from the beigining address of  the buffer if allowed.
 */

#ifndef MAPPORT_CBUFFER_H_
#define MAPPORT_CBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

class CBuffer {
 public:
  struct tVec {
    const char *p;
    uint32_t len;
  };

 public:
  CBuffer(uint32_t buf_size_ = 4096);
  virtual ~CBuffer();

  // This might be a bug! TODO
  void resize(uint32_t size) {
    void *p = realloc(p_, size);
    if (p)  // if p is null, then nothing changed...
    {
      p_ = (char *)p;
      size_ = size;
    }
  }

  uint32_t len() { return tail_ - head_; }

  uint32_t left() { return size_ - len(); }

  bool empty() { return head_ == tail_; }

  bool full() { return len() == size_; }

  void clear() { head_ = tail_ = 0; }

  bool cycleWrite(const char *p, uint32_t len);

  const char *cycleRead(uint32_t &len);

  void commitRead(uint32_t len) { head_ += len; }

  void commitWrite(uint32_t len) { tail_ += len; }

  void getLeftVec(struct iovec v[2]);

  void getDataVec(struct iovec v[2]);

 public:
  char *p_;        // pointer to the start of the buffer memory
  uint32_t size_;  // size of the buffer, in bytes

  // The head_ and tail_ are the counter indicating total bytes read out from
  // the buffer and written into the buffer respectively.They both increment
  // monotonically from 0. The actual address of the data region is calculated
  // as follows: head_address = p_ + head_ % size_ , tail_address = p_ + tail_ %
  // size_
  uint32_t head_;  // start index of the buffer region containing data
  uint32_t tail_;  // stop index of the buffer region containing data
};

#endif /* MAPPORT_CBUFFER_H_ */
