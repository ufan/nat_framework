/*
 * strategy_shared_comm.h
 *
 *  Created on: 2017年10月19日
 *      Author: hongxu
 */

#ifndef SRC_INCLUDE_STRATEGY_SHARED_COMM_H_
#define SRC_INCLUDE_STRATEGY_SHARED_COMM_H_

#include <stdint.h>
#include <string.h>

#include "CHashTable.h"
#include "MurmurHash2.h"

#define SHARED_STG_HASH_BASE 2000
#define SHARED_STG_HASH_DEEP 5
#define SHARED_STG_HASH_SHMKEY 0x20171020

#define SHARED_STG_KEY_TYPE uint32_t
#define SHARED_STG_NAME_MAX_SIZE 128

#define SHARED_STG_HASH(p, len) MurmurHash2(p, len, 0x2017)

// Meta data about running strategy (a record in strategy table)
// Abnormal exit status of the strategy is not recorded here, the pid should be
// checked
struct tStrategyNode {
  char name[SHARED_STG_NAME_MAX_SIZE];
  uint32_t start_time;  // when strategy is started
  uint32_t is_exit;   // flag controlling and indicating normal exit of strategy
  uint32_t do_trade;  // flag controlling and indicating do trading or not
  uint64_t userdata;

  void setName(const char* p) {
    strncpy(name, p, sizeof(name));
    name[sizeof(name) - 1] = '\0';
  }

  const char* getName() const { return name; }
};

#define SHARED_STG_TABLE COM::CHashTable<tStrategyNode, SHARED_STG_KEY_TYPE>
typedef SHARED_STG_TABLE::tNode SHARED_STG_TNODE;

#define SHARED_STG_INIT_TABLE(t) \
  (t).init(SHARED_STG_HASH_BASE, SHARED_STG_HASH_DEEP, SHARED_STG_HASH_SHMKEY)

#define SHARED_STG_STR_TO_KEY(s)                                 \
  SHARED_STG_HASH(s.data(), (s.size() < SHARED_STG_NAME_MAX_SIZE \
                                 ? s.size()                      \
                                 : SHARED_STG_NAME_MAX_SIZE - 1))

#endif /* SRC_INCLUDE_STRATEGY_SHARED_COMM_H_ */
