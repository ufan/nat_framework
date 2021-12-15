/*
 * CHashTable.h
 *
 *  Created on: 2017年10月19日
 *      Author: hongxu
 */

#ifndef SRC_COMMON_CHASHTABLE_H_
#define SRC_COMMON_CHASHTABLE_H_

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace COM {

template <class VALTYPE, class KEYTYPE = uint64_t>
class CHashTable {
 public:
  struct tNode {
    KEYTYPE key;
    VALTYPE val;
  };

  CHashTable() : basesize_(NULL), p_node_(NULL), shmkey_(0) {}
  virtual ~CHashTable() {
    if (basesize_) {
      delete[] basesize_;
    }

    if (shmkey_ == 0) {
      if (p_node_) {
        delete[] p_node_;
      }
    } else {
      if (p_node_) {
        shmdt((void *)p_node_);
      }
    }
  }

  bool init(uint32_t base, uint32_t deep = 5, uint32_t shmkey = 0) {
    uint32_t tot_size = calcSize(base, deep);
    if (shmkey == 0)  // use dynamic allocated memory
    {
      p_node_ = new tNode[totcnt_];
      memset(p_node_, 0, tot_size);
    } else {  // use shared memory
      // Check if the specified shm existence
      int shmid = shmget(shmkey, 0, SHM_R | SHM_W);
      if (shmid == -1) {
        if (errno == ENOENT)  // create one, not exist before
        {
          shmid = shmget(shmkey, tot_size, SHM_R | SHM_W | IPC_CREAT);
          if (shmid == -1) return false;
        } else
          return false;
      }

      // get the pointer to shm in the process addr space
      p_node_ = (tNode *)shmat(shmid, NULL, 0);
      if ((void *)p_node_ == (void *)(-1)) {
        p_node_ = NULL;
        return false;
      }
    }

    // keep a copy the shm identification key
    shmkey_ = shmkey;
    return true;
  }

  uint32_t calcSize(uint32_t base, uint32_t deep) {
    if (basesize_) {
      delete[] basesize_;
    }
    basesize_ = new uint32_t[deep];
    deep_ = deep;

    if (base % 2 == 0) ++base;

    totcnt_ = 0;
    for (uint32_t i = 0; i < deep; ++i) {
      while (!isPrime(base)) base += 2;
      basesize_[i] = base;
      totcnt_ += base;
      base += 2;
    }
    return sizeof(tNode) * totcnt_;
  }

  bool isPrime(uint32_t num) {
    if (num != 2 && num % 2 == 0) return false;
    for (uint32_t i = 3; i * i <= num; i += 2) {
      if (num % i == 0) return false;
    }
    return true;
  }

  tNode *get(KEYTYPE key) {
    tNode *p = p_node_;
    for (uint32_t i = 0; i < deep_; ++i) {
      uint32_t j = (uint32_t)(key % basesize_[i]);
      if (p[j].key == key) return &(p[j]);
      p += basesize_[i];
    }
    return NULL;
  }

  tNode *insert(KEYTYPE key, VALTYPE &val) {
    tNode *p = p_node_;
    tNode *p_empty = NULL;

    for (uint32_t i = 0; i < deep_; ++i) {
      uint32_t j = (uint32_t)(key % basesize_[i]);
      if (p[j].key == key) {
        p[j].val = val;
        return &(p[j]);
      } else if (!p_empty && p[j].key == 0) {
        p_empty = &(p[j]);
      }

      p += basesize_[i];
    }

    if (p_empty) {
      p_empty->key = key;
      p_empty->val = val;
      return p_empty;
    }
    return NULL;
  }

  bool del(KEYTYPE key) {
    tNode *p = p_node_;
    for (uint32_t i = 0; i < deep_; ++i) {
      uint32_t j = (uint32_t)(key % basesize_[i]);
      if (p[j].key == key) {
        p[j].key = 0;
        return true;
      }
      p += basesize_[i];
    }
    return false;
  }

  void first() { curpos_ = 0; }

  tNode *next() {
    if (curpos_ < totcnt_) {
      return &(p_node_[curpos_++]);
    }
    return NULL;
  }

 public:
  uint32_t deep_;
  uint32_t *basesize_;
  tNode *p_node_;
  uint32_t curpos_;
  uint32_t totcnt_;
  uint32_t shmkey_;
};

}  // end namespace COM

#endif /* SRC_COMMON_CHASHTABLE_H_ */
