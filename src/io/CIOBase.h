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

#include "CTimer.h"
#include "Logger.h"
#include "compiler.h"
#include "ioprotocol.h"
using namespace std;

#define IO_FILE_SUFFIX "_io_"

class CIOBase {
 public:
  CIOBase(uint32_t page_size = IO_PAGE_SIZE);
  virtual ~CIOBase();

  // set the file path prefix
  void setPrefix(string prefix) { prefix_ = prefix + IO_FILE_SUFFIX; }

  // set size of a single mmap file (i.e., page in nat_framework jargon)
  void setPageSize(uint32_t page_size) { size_ = page_size; }

  // get mmap file name prefix (i.e., without index number)
  string getPrefix() { return prefix_; }

  void setMemMode(bool lock) { is_lock_ = lock; }

  // return the last mmap file index with the same prefix_ name,
  // i.e., max(index_num). return -1 if no file with the pattern exists.
  int getTailNum();

  // return the first mmap file index with the same prefix_, i.e. min
  // (index_num). return -1 if no file with the pattern exists.
  int getHeadNum();

  // get a sorted vector of all mmap file index with the same prefix_ name.
  void getPageNum(vector<int> &vno);

  // page_no=-1 to return the tail index. otherwise return the first index
  // larger or equal than page_no. if page_no is out of range, return -1.
  int getUpperPageNo(int page_no);

  // open a new mmap file
  bool load(int num, bool is_write = false);

  // unmap and release resources
  void unload();

  bool checkFrameValid(void *frame) {
    return FRAME_HEAD_MAGIC == ((tFrameHead *)frame)->magic;
  }

  bool checkPageHead(void *head) {
    return ((tPageHead *)head)->version == PAGE_HEAD_VER_1 &&
           ((tPageHead *)head)->size == size_;
  }

  // checking the validility of the mmap file
  bool hasLoad() { return fd_ >= 0; }

  char *getRawBuffer(uint32_t size) {
    size = size_;
    return buf_;
  }

 protected:
  // mmap file name patter: prefix_ + index_num, index starts from 0
  string prefix_;

  char *buf_;      // start addr of the portion of mapped memory
  uint32_t size_;  // size of the mmap file (i.e., page size in nat_framework
                   // jargon), default 128 MB
  int fd_;         // file descriptor of the mmap file

  // flag indicating lock the mmap memory from swapping to the disk default true
  bool is_lock_;
};

#endif /* SRC_IO_CIOBASE_H_ */
