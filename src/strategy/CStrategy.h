/*
 * CStrategy.h
 *
 *  Created on: May 25, 2018
 *      Author: hongxu
 *
 * Comment by Yong:
 * CStrategy is designed to be inherited by the user to implement trading
 * strategies.
 *
 * Minimum Running:
 * 1. init()
 * 2. run()
 * 3. release()
 *
 * User should override the callback slot functions to implement his strategy
 * ideas. Two groups of callbacks are provided:
 * 1. on_xxx: high-level, interface methods, to be used by simple strategy
 * 2. sys_on_xxx: low-level, protected methods, to be used by complicated
 * strategy
 * Both groups of methods are called back at the same time slot of the running
 * process. But, sys_on_xxx group is always invoked before the on_xxx group.
 *
 */

#ifndef SRC_STRATEGY_CSTRATEGY_H_
#define SRC_STRATEGY_CSTRATEGY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ATStructure.h"
#include "BarHelper.h"
#include "BarMaker.h"
#include "CSystemIO.h"
#include "IMDHelper.h"
#include "ITDHelper.h"
#include "RiskStg.h"
using namespace std;

template <class K, class V>
using umap = unordered_map<K, V>;

class CStrategy {
 public:
  CStrategy();
  virtual ~CStrategy();

  /**
   * @name High-level callback slots
   * These callbacks are used by high-end user to implement some simple
   * strategy. 'Simple' in the sense that customization ability is limited to
   * the pre-designed model parameter adjusting. These are also exposed as
   * interface of the strategy, thus in 'public' domain.
   * @{
   */
  virtual void on_tick(const UnitedMarketData*) {}
  virtual void on_bar(const Bar*) {}
  virtual void on_time(long nano) {}
  virtual void on_rtn(const tRtnMsg*) {}
  virtual void on_switch_day(string day) {}
  // callback for user defined message from system io
  virtual void on_msg(const char* p, uint32_t len) {}

  /**
   * @name Basic methods for running a strategy
   * @{
   */
  bool init(string config_file);
  bool initStr(string config_content);
  void run();
  void stop();
  void release();

  /**
   * @name Utility methods for strategy implementation
   * @{
   */
  // subscribe to instruments managed by this strategy
  bool subscribe(string instruments);
  bool subsBar(string instrument, long interval_min);
  void setIntraDayBaseSec(long sec) { BarHelper::setIntraDayBaseSec(sec); }

  // send request for insert/delete order to td engine
  int sendOrder(uint32_t instr_hash, double price, int vol, int dir, int off,
                int acc_idx = 0);
  int sendOrder(string instr, double price, int vol, int dir, int off,
                int acc_idx = 0) {
    return sendOrder(INSTR_STR_TO_HASH(instr), price, vol, dir, off, acc_idx);
  }
  int cancelOrder(int order_id);

  // read latest tick data from md engine
  const UnitedMarketData* readMd();

  // set starting timestamp for reading quote, default is the start time of this
  // strategy
  void setReadPos(long nano) { p_md_helper_->setReadPos(nano); }

  /**
   * @name Getters
   * @{
   */
  const tLastTick* getLastTick(uint32_t instr_hash) {
    auto itr = instr_info_map_.find(instr_hash);
    return itr != instr_info_map_.end() ? &(itr->second.lst_tick) : nullptr;
  }
  const tLastTick* getLastTick(string instr) {
    return getLastTick(INSTR_STR_TO_HASH(instr));
  }
  int getOrderTrackCnt() { return p_td_helper_->getOrderTrackCnt(); }
  tOrderTrack& getOrderTrack(int idx) {
    return p_td_helper_->getOrderTrack(idx);
  }
  string getTradingDay() { return trading_day_; }
  const RiskStg* getAccountObj() { return p_risk_stg_.get(); }
  /**
   * @}
   */

 private:
  bool loadConfig(string config_content);
  void pre_on_switch_day(string day);

  void processTick(const UnitedMarketData* pmd);
  void processSysIO();

  void barOnFinish();

 protected:
  /**
   * @name Low-level callback slots
   * These callbacks are used by strategy designer to implement a
   * low-level trading model, which will be later used by high-end users
   * to further implement their strategy based on the low-end model.
   * The high-end user is not supposed to 'see' these model implementation
   * details, thus in the 'protected' domain.
   * See the design of CExecuteStrategy.
   * @{
   */
  virtual void sys_on_start() {}
  virtual void sys_on_tick(const UnitedMarketData*) {}
  virtual void sys_on_time(long nano) {}
  virtual void sys_on_rtn(const tRtnMsg*) {}
  virtual void sys_on_switch_day(string day) {}
  virtual void sys_on_stop() {}
  virtual void sys_on_subscribe(string instr) {}
  /**
   * @}
   */

 private:
  // hash id from name, the unique id of this running strategy
  // Name is read from config file. The same name is used to identify td/md
  // helper, risk_stg and other objects working for this strategy.
  int self_id_ = 0;

  // collection of subscribed instruments, which contains the base infoand
  // the last tick
  umap<uint32_t, tSubsInstrInfo> instr_info_map_;

  unique_ptr<IMDHelper> p_md_helper_;  // for communincation with md engine
  unique_ptr<ITDHelper> p_td_helper_;  // for communication with td engine
  unique_ptr<RiskStg> p_risk_stg_;     // for risk account management

  // collection of bar makers, each elemnt matching one of the subscribed
  // instruments
  vector<BarMaker> subs_bar_;

  // flags controlling the strategy running status
  volatile uint32_t* p_is_exit_ = nullptr;
  volatile uint32_t* p_do_trade_ = nullptr;

  string trading_day_;   // current trading day
  string acc_save_day_;  // previous trading day, aimed to be saved

  // system io reader
  unique_ptr<CRawIOReader> p_sys_io_reader_;
};

#endif /* SRC_STRATEGY_CSTRATEGY_H_ */
