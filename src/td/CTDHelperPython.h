/*
 * CTDHelperPython.h
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 *
 * Comment by Yong:
 * Another helper for replay purpose.
 * It invokes an python module where defines the methods simulating the real
 * exchange.
 *
 */

#ifndef SRC_TD_CTDHELPERPYTHON_H_
#define SRC_TD_CTDHELPERPYTHON_H_

#include "ITDHelper.h"
#include "PyExtExch.h"

class CTDHelperPython : public ITDHelper {
 public:
  using ITDHelper::ITDHelper;
  virtual ~CTDHelperPython() { release(); }

  bool init(const json& j_conf);

  void doSendOrder(int track_id) { exch_.sendOrder(getOrderTrack(track_id)); }

  void doDelOrder(int track_id) {
    exch_.delOrder(getOrderTrack(track_id).order_ref);
  }

  const tRtnMsg* doGetRtn();

  void release() { exch_.release(); }

  virtual bool qryTradeBaseInfo() { return exch_.qryTradeBaseInfo(); }

  virtual bool qryOrderTrack() { return exch_.qryOrderTrack(order_track_); }

  virtual void on_tick(const UnitedMarketData* pmd) { exch_.on_tick(pmd); }
  virtual void on_time(long nano) { exch_.on_time(nano); }
  virtual void on_switch_day(string day) {
    closeOrderTrack();
    exch_.on_switch_day(day);
  }

 protected:
  CPyExtTdExch exch_;
};

#endif /* SRC_TD_CTDHELPERPYTHON_H_ */
