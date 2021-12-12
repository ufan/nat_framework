/*
 * CStrategyProcess.h
 *
 *  Created on: 2018年5月9日
 *      Author: sky
 *
 * Comment by Yong
 * CStrategyProcess, together with CStrategyBase, implement the same function as
 * CStrategy. The strategy is separated into two parts: 1) the running
 * environment managed by CStrategyProcess; 2) the strategy design interface
 * CStrategyBase. This separation provides a better strategy desgin interface
 * and user can now be more focused on the 'strategic' part of the design.
 *
 * CStrategyBase is the base class, which provides the implementation interface
 for strategy designer. The on_xxx group of virtual functions are the callback
 slots by CStrategyProcess. Users should override these methods to implement
 their strategy.
 *
 * CStrategyProcess is an aggregation of static methods, which
 * defines the running procedure and sequence in the life of a strategy.
 * The communication with TD and MD engine, the record of subscribed
 * instruments, the list of bar makers is also managed in it. Most important of
 * all, a list of CStrategyBase is also managed by CStrategyProcess. This is a
 * big improvement comparing to CStrategy, since now multiple strategies can be
 * chained together instead of just one strategy. Each strategy has its own
 * account/risk management. Due to this 1-to-N scheme, each strategy has an
 * identification id, which is just the index of its position is the strategy
 * list.
 * There are other 1-to-N mapping (in instruments list and bar maker list):
 * 1. Each instrument may subscribed by multiple base strategies.
 * 2. Each base strategy may subscribe multiple instruments, which in turn means
 * its risk account may manage multiple instruments.
 * 3. Each bar maker, which is specified by a specific instrument and an time
 * interval, may be consumed by multiple instruments.
 * All these mapping are recorded in CStrategyProcess. In this sense,
 * CStrategyProcess is designed to deal with multi-strategy, while CStrategy is
 * designed to deal with single-strategy.
 *
 * The basic running environment is established as follows:
 * 1) CStrategyProcess::init()
 * 2) Create and activate strategies, and subscribe to instruments for each
 *    strategy
 *    i) stg->activate()
 *   ii) stg->subscribe() or stg->subsBar()
 * 3) CStrategyProcess::run()
 * 4) CStrategyProcess::release()

 * NOTE: Message from system io is not processed in CStrategyProcess, while in
 CStrategy, user has the option to process the message from system io.
 */

#ifndef STRATEGY_CSTRATEGYPROCESS_H_
#define STRATEGY_CSTRATEGYPROCESS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ATStructure.h"
#include "BarMaker.h"
#include "IMDHelper.h"
#include "ITDHelper.h"
#include "RiskStg.h"
using namespace std;

template <class K, class V>
using umap = unordered_map<K, V>;

class CStrategyBase;

class CStrategyProcess {
  struct tSubscribedInstrInfo {
    tSubsInstrInfo instr_info;
    vector<CStrategyBase*> stgs;
  };

 public:
  static bool init(string config_file);
  static bool initStr(string config_content);
  static bool loadConfig(string config_content);
  static int add(CStrategyBase* stg, int profile_id = 0);
  static void run();
  static void stop();
  static void release();
  static bool subscribe(CStrategyBase* stg, string instruments);
  static bool subsBar(CStrategyBase* stg, string instrument, long interval_min);

  static int sendOrder(const char* instr, uint32_t instr_hash, double price,
                       int vol, int dir, int off, int acc_idx, int stgid) {
    return p_td_helper_->sendOrder(instr, price, vol, dir, off, acc_idx, stgid,
                                   instr_hash);
  }

  static int cancelOrder(int order_id) {
    return p_td_helper_->deleteOrder(order_id);
  }

  static const tLastTick* getLastTick(uint32_t instr_hash) {
    auto itr = instr_info_map_.find(instr_hash);
    return itr != instr_info_map_.end() ? &(itr->second.instr_info.lst_tick)
                                        : nullptr;
  }

  static const tLastTick* getLastTick(string instr) {
    return getLastTick(INSTR_STR_TO_HASH(instr));
  }

  static void setReadPos(long nano) { p_md_helper_->setReadPos(nano); }

  static int getOrderTrackCnt() { return p_td_helper_->getOrderTrackCnt(); }

  static tOrderTrack& getOrderTrack(int idx) {
    return p_td_helper_->getOrderTrack(idx);
  }

  static tSubsInstrInfo* getSubsInstrInfo(uint32_t instr_hash) {
    auto itr = instr_info_map_.find(instr_hash);
    if (itr != instr_info_map_.end()) {
      return &(itr->second.instr_info);
    }
    return nullptr;
  }

  static void pre_on_switch_day(string day);

  static string getTradingDay() { return trading_day_; }

 private:
  static void on_tick(const UnitedMarketData*);
  static void on_time(long nano);
  static void on_rtn(const tRtnMsg*);
  static void barOnFinish();

 private:
  static IMDHelper* p_md_helper_;
  static ITDHelper* p_td_helper_;
  static vector<CStrategyBase*> strategy_list_;
  static umap<uint32_t, tSubscribedInstrInfo> instr_info_map_;
  static vector<pair<BarMaker, vector<CStrategyBase*>>> subs_bar_info_;

 public:
  static volatile uint32_t* p_is_exit_;
  static volatile uint32_t* p_do_trade_;
  static string trading_day_;
  static string acc_save_day_;
};

class CStrategyBase {
 public:
  virtual ~CStrategyBase() {}

  /**
   * @name Callback slots, same as CStrategy
   * @{
   */
  virtual void on_tick(const UnitedMarketData*) {}
  virtual void on_time(long nano) {}
  virtual void on_rtn(const tRtnMsg*) {}
  virtual void on_bar(const Bar*) {}
  virtual void on_switch_day(string day) {}

  /**
   * @name Methods for initialize this strategy in the main function
   * @{
   */
  int activate(int profile_id = 0) {
    return CStrategyProcess::add(this, profile_id);
  }
  bool subscribe(string instruments) {
    return CStrategyProcess::subscribe(this, instruments);
  }
  bool subsBar(string instrument, long interval_min) {
    return CStrategyProcess::subsBar(this, instrument, interval_min);
  }

  /**
   * @name Methods used by strategy to issue and delete an order
   * @{
   */
  int sendOrder(uint32_t instr_hash, double price, int vol, int dir, int off,
                int acc_idx = 0) {
    if (*CStrategyProcess::p_do_trade_) {
      auto info = CStrategyProcess::getSubsInstrInfo(instr_hash);
      if (info) {
        int ret = 0;
        if (0 == (ret = p_risk_stg_->check(dir, off, price, vol, info))) {
          ret = CStrategyProcess::sendOrder(info->base_info.instr, instr_hash,
                                            price, vol, dir, off, acc_idx,
                                            self_id_);
          p_risk_stg_->onNew(dir, off, price, vol, instr_hash,
                             info->lst_tick.exch_time);
        }
        return ret;
      }
    }
    return -1;
  }

  int sendOrder(string instr, double price, int vol, int dir, int off,
                int acc_idx = 0) {
    return sendOrder(INSTR_STR_TO_HASH(instr), price, vol, dir, off, acc_idx);
  }

  int cancelOrder(int order_id) {
    return CStrategyProcess::cancelOrder(order_id);
  }

  /**
   * @name Get the risk account object of this strategy
   */
  const RiskStg* getAccountObj() { return p_risk_stg_.get(); }

 private:
  int self_id_ = 0;  // unique id assigned by CStrategyProcess and kept as
                     // stg_id in the order track of td engine

  // the risk account object owned by this strategy
  unique_ptr<RiskStg> p_risk_stg_;

  friend class CStrategyProcess;
};

#endif /* STRATEGY_CSTRATEGYPROCESS_H_ */
