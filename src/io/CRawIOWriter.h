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

/*
 * CRawIOWriter focus on writing in one process with exclusive write permission
 * for the Pages managed by it.
 * It addes the underlying Page Format checking, comparing CIOBase.
 * It also defines the interface for the writing new frame data into Page.
 * It's not thread-safe, designed to be used in single-thread environment.
 */

class CRawIOWriter : public CIOBase {
 public:
  CRawIOWriter(uint32_t page_size = IO_PAGE_SIZE);
  virtual ~CRawIOWriter();

  bool init(string path);
  bool write(const void* data, uint32_t len);

  char* prefetch(uint32_t len);
  void commit();

  // overwrites CIOBase::load
  bool load(int num);
  // create new page
  bool createNextPage();

 protected:
  int page_num_;            // the current index of Page
  uint32_t prefetch_tail_;  // the end position of the prefetched frame in Page
                            // memory
  uint32_t page_size_;      // the default size of newly-created page
  bool is_test_lock_onload_;  // flag trying to lock the Page file when loading
                              // it
};

//===================================================================================

/*
 * CSafeRawIOWriter is built upon the CRawIOWriter with a focus on the data
 * synchronization in multi-threading environment. In another term,
 * CSafeRawIOWriter is thread-safe, while CRawIOWriter is not. On the other
 * hand, it will not lock the file on load. It must be used in a single process.
 * Using CSafeRawIOWriter in different process to manage the same set of Pages
 * may have undefined behavior.
 */

class CSafeRawIOWriter : public CRawIOWriter {
 public:
  CSafeRawIOWriter(uint32_t page_size = IO_PAGE_SIZE)
      : CRawIOWriter(page_size) {
    // the mmap file is not locked between process
    is_test_lock_onload_ = false;
  }

  // Thread-safe methods
  bool init(string path);                      // atomic
  bool write(const void* data, uint32_t len);  // atomic

  // after prefetch, the resource is locked in a thread until commit
  // prefetch and commit together compose an atomic operation
  char* prefetch(uint32_t len);  // not atomic, only acquire
  void commit();                 // not atomic, only release

  void discard();  // clear the atomic flag
};

#endif /* SRC_IO_CRAWIOWRITER_H_ */
