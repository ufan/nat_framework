/*
 * CTradeBaseInfo.h
 *
 *  Created on: May 28, 2018
 *      Author: hongxu
 */

/*
 * Central repository of all Instruments of a process.
 * This is not a repository shared among multiple processes.
 * All data resides in memory and only accessible to the process itself.
 * Thus, each process may maintain its own copy of the trade base information.
 * The source of this information is TD engine. TD retrieve these data from
 * trade front. However, in the same trading day, the information need not to be
 * updated. Thus, it's also proper to retrieve this information from second-hand
 * source.
 *
 * Commented by Yong.Z
 */

#ifndef SRC_TD_CTRADEBASEINFO_H_
#define SRC_TD_CTRADEBASEINFO_H_

#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ATStructure.h"
#include "IOCommon.h"
using namespace std;

using switchday_fn = function<void(string)>;

class CTradeBaseInfo {
 public:
  static void set(const tIOTDBaseInfo *p);

  static bool update(const tIOTDBaseInfo *p);

  static string toSysIOStruct(int to, int source, int back_word);

  static vector<tInstrumentInfo> getInstrInProduct(string product);

  static std::set<string> productToInstrSet(const vector<string> &product);
  static std::set<string> productOrInstrumentToInstrSet(
      const vector<string> &product);

  static string getTradingDay() { return CTradeBaseInfo::trading_day_; }
  static void setTradingDay(string day) { CTradeBaseInfo::trading_day_ = day; }

  static void setInitFlag(bool flag) { is_init_ = flag; }

  static void addInstrInfo(tInstrumentInfo info);

  static void clearInstrInfo() { instr_info_.clear(); }

  static void callOnSwitchDayCb();

  static const tInstrumentInfo *getInstrInfo(uint32_t hash) {
    auto itr = instr_info_.find(hash);
    if (itr != instr_info_.end()) {
      return &(itr->second);
    }
    return nullptr;
  }

 public:
  static bool is_init_;        // flag for intialization
  static string trading_day_;  // current trading day

  // collection of all instruments currently listed on market
  static unordered_map<uint32_t, tInstrumentInfo> instr_info_;

  // callback function for day-switching, default in null
  static switchday_fn switch_day_cb_;
};

inline void setSwitchDayCallBack(switchday_fn fn) {
  CTradeBaseInfo::switch_day_cb_ = fn;
}

int exchangeStr2int(string exch);

const char *exchangeint2str(int exch);

#endif /* SRC_TD_CTRADEBASEINFO_H_ */
