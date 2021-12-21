/*
 * CTDHelperFake.cpp
 *
 *  Created on: 2018年5月14日
 *      Author: hongxu
 */

#include "CTDHelperFake.h"

#include "MurmurHash2.h"
#include "compiler.h"

extern int g_replay_md_fake_wait_count;

// fill in the basic information of an rtn msg, fine details can be modified
// later. Also, increase the wait count of MD replay.
static void fillRtnMsg(tRtnMsg &rtn, tOrderTrack &ot, int id) {
  rtn.instr_hash = ot.instr_hash;
  memcpy(rtn.instr, ot.instr, sizeof(rtn.instr));
  rtn.price = ot.price;
  rtn.vol = ot.vol;
  rtn.dir = ot.dir;
  rtn.off = ot.off;
  rtn.order_ref = id;
  rtn.local_id = id;

  // increase the wait count of md replay
  // replay, see CMDHelperReplayCtpDump and CMDHelperReplayIO
  g_replay_md_fake_wait_count++;
}

// send a new order to the fake market and entered into the auction queue
void CTDHelperFake::doSendOrder(int track_id) {
  tOrderTrack &ot = getOrderTrack(track_id);
  eng_order_track_.push_back(ot);
  if (unlikely(ot.dir != AT_CHAR_Buy && ot.dir != AT_CHAR_Sell)) {
    rtn_msg_queue_.emplace();
    tRtnMsg &err_msg = rtn_msg_queue_.back();
    fillRtnMsg(err_msg, ot, track_id);
    err_msg.msg_type = ODS(REJECT);
    eng_order_track_.back().status |= ODS(REJECT);
    strcpy(err_msg.msg, "direction is invalid.");
    return;
  }

  rtn_msg_queue_.emplace();
  tRtnMsg &send_msg = rtn_msg_queue_.back();
  fillRtnMsg(send_msg, ot, track_id);
  send_msg.msg_type = ODS(TDSEND);

  rtn_msg_queue_.emplace();
  tRtnMsg &acc_msg = rtn_msg_queue_.back();
  fillRtnMsg(acc_msg, ot, track_id);
  acc_msg.msg_type = ODS(ACCEPT);

  rtn_msg_queue_.emplace();
  tRtnMsg &market_acc_msg = rtn_msg_queue_.back();
  fillRtnMsg(market_acc_msg, ot, track_id);
  market_acc_msg.msg_type = ODS(MARKET_ACCEPT);

  eng_order_track_.back().status |=
      ODS(TDSEND) | ODS(ACCEPT) | ODS(MARKET_ACCEPT);

  // insert this order into auction queue and do the auction immediately
  // since auction on existing orders in the queue already took place when
  // tick is updated
  queued_track_id_.insert(track_id);
  auto itr = last_tick_wall_.find(send_msg.instr_hash);
  if (itr != last_tick_wall_.end()) {
    if (makeMatch(track_id, itr->second)) {
      queued_track_id_.erase(track_id);
    }
  }
}

// delete an exiting order
void CTDHelperFake::doDelOrder(int track_id) {
  tOrderTrack &ot = getEngOrderTrack(track_id);
  rtn_msg_queue_.emplace();
  tRtnMsg &msg = rtn_msg_queue_.back();
  fillRtnMsg(msg, ot, track_id);

  if ((ot.status & ODS(CLOSED)) || ot.vol <= ot.vol_traded) {
    ot.status |= ODS(CANCEL_REJECT);
    msg.msg_type = ODS(CANCEL_REJECT);
  } else {
    ot.status |= ODS(CANCELED);
    msg.msg_type = ODS(CANCELED);
    msg.vol = ot.vol - ot.vol_traded;
  }

  queued_track_id_.erase(track_id);
}

// pop out the first rtn msg in the queue
const tRtnMsg *CTDHelperFake::doGetRtn() {
  if (not rtn_msg_queue_.empty()) {
    hold_result_ = rtn_msg_queue_.front();
    rtn_msg_queue_.pop();
    return &hold_result_;
  }
  return nullptr;
}

/**
 * @brief Return the middle value of a, b, c
 * a < b < c, return b
 * a < c <= b, return c
 * c <= a < b, return a
 * c < b <= a, return b
 * b <= a < c, return a
 * b <= c <= a, return c
 */
static double median(double a, double b, double c) {
  if (a < b) {
    if (b < c)
      return b;
    else if (a < c)
      return c;
    else
      return a;
  } else {
    if (c < b)
      return b;
    else if (a < c)
      return a;
    else
      return c;
  }
}

/**
 * @brief Auction of an order based on the track id against the tick price.
 * Return true if this order is finished in this auction, otherwise false.
 */
bool CTDHelperFake::makeMatch(int track_id, CTDHelperFake::tTick &tick) {
  // get the order info
  tOrderTrack &ot = getEngOrderTrack(track_id);
  int left = ot.vol - ot.vol_traded;
  if (ot.dir == AT_CHAR_Buy) {
    if (tick.ask <= ot.price && tick.ask_vol > 0 && left > 0) {
      int trade = tick.ask_vol < left ? tick.ask_vol : left;
      tick.ask_vol -= trade;
      ot.vol_traded += trade;
      ot.status |= ODS(EXECUTION);
      rtn_msg_queue_.emplace();
      tRtnMsg &msg = rtn_msg_queue_.back();
      fillRtnMsg(msg, ot, track_id);
      msg.vol = trade;
      msg.price = median(tick.ask, tick.last_px, ot.price);
      msg.msg_type = ODS(EXECUTION);
    }
  } else if (ot.dir == AT_CHAR_Sell) {
    if (tick.bid >= ot.price && tick.bid_vol > 0 && left > 0) {
      int trade = tick.bid_vol < left ? tick.bid_vol : left;
      tick.bid_vol -= trade;
      ot.vol_traded += trade;
      ot.status |= ODS(EXECUTION);
      rtn_msg_queue_.emplace();
      tRtnMsg &msg = rtn_msg_queue_.back();
      fillRtnMsg(msg, ot, track_id);
      msg.vol = trade;
      msg.price = median(tick.bid, tick.last_px, ot.price);
      msg.msg_type = ODS(EXECUTION);
    }
  }

  // this order is finished in this auction
  if (ot.vol_traded >= ot.vol) {
    ot.status |= ODS(CLOSED);
    return true;
  }
  return false;
}

/**
 * @details Auction and trade based on latest market tick, unfilled orders are
 * matched based on the FIFO principle. Tick is updated with the first bid/ask
 * price, ignore the other 4 prices. This tick will be used as the reference of
 * auction for new orders issued before the next tick.
 * @param[in] pmd Latest tick from market
 */
void CTDHelperFake::on_tick(const UnitedMarketData *pmd) {
  tTick &tick = last_tick_wall_[pmd->instr_hash];
  tick.last_px = pmd->last_px;
  tick.ask = pmd->ask_px;
  tick.ask_vol = pmd->ask_vol;
  tick.bid = pmd->bid_px;
  tick.bid_vol = pmd->bid_vol;

  for (auto itr = queued_track_id_.begin();
       itr != queued_track_id_.end() && (tick.ask_vol || tick.bid_vol);) {
    tOrderTrack &ot = getEngOrderTrack(*itr);
    if (strcmp(ot.instr, pmd->instr_str) == 0) {
      if (makeMatch(*itr, tick)) {
        queued_track_id_.erase(itr++);
        continue;
      }
    }
    itr++;
  }
}

void CTDHelperFake::on_switch_day(string day) {
  closeOrderTrack();
  eng_order_track_.clear();
  last_tick_wall_.clear();
  queued_track_id_.clear();
}
