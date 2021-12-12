/*
 * ITDHelper.h
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#ifndef SRC_TD_ITDHELPER_H_
#define SRC_TD_ITDHELPER_H_

#include <memory>
#include <string>
#include <vector>

#include "ATStructure.h"
#include "Logger.h"
#include "MurmurHash2.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class ITDHelper {
 public:
  ITDHelper(string name);
  virtual ~ITDHelper() {}

  // interface functions
  int sendOrder(const char* instr, double price, int vol, int dir, int off,
                int acc_id, int stgid, uint32_t instr_hash);
  int deleteOrder(int id);
  const tRtnMsg* getRtn();

  int getOrderTrackCnt() { return order_track_.size(); }
  tOrderTrack& getOrderTrack(int idx) { return order_track_[idx]; }

  /**
   * @brief Get strategy id from the return message
   * @return Always 0 for simple strategy derived from CStrategy.
   *         For strategy derived from CStrategyBase, return the index of this
   *         strategy in the CStrategyProcess's list of strategies.
   */
  int getStgIdFromRtnMsg(const tRtnMsg* prtn) {
    return getOrderTrack(prtn->local_id).stg_id;
  }
  void closeOrderTrack();

  // TODO to be implemeted in child, real operation, should be private
  virtual bool init(const json& j_conf) = 0;
  virtual void doSendOrder(int track_id) = 0;
  virtual void doDelOrder(int track_id) = 0;
  virtual const tRtnMsg* doGetRtn() = 0;

  // default implementation, to be override in child
  virtual void release() {}
  virtual bool qryTradeBaseInfo() { return false; }
  virtual bool qryOrderTrack() { return false; }

  virtual void on_tick(const UnitedMarketData* pmd) {}
  virtual void on_time(long nano) {}
  // reset track records on a new trade day
  virtual void on_switch_day(string day) { closeOrderTrack(); }

 public:
  int sendOrder(string instr, double price, int vol, int dir, int off,
                int acc_id, int stgid = 0) {
    return sendOrder(instr.c_str(), price, vol, dir, off, acc_id, stgid,
                     INSTR_STR_TO_HASH(instr));
  }

  bool initStr(string conf) { return init(json::parse(conf)); }

 protected:
  // records of issued orders, default size is 1024*2 but can dynamically
  // expanded. The order can be identified by the index in this collection
  // locally.
  vector<tOrderTrack> order_track_;

  string name_;  // Name of this helper (or client), must be unique in the
                 // system both online/offline. An id will be generated from
                 // this name.
};

typedef unique_ptr<ITDHelper> ITDHelperPtr;

#endif /* SRC_TD_ITDHELPER_H_ */
