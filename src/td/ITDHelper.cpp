/*
 * ITDHelper.cpp
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#include "ITDHelper.h"

#include <string.h>

#include "MurmurHash2.h"

ITDHelper::ITDHelper(string name) : name_(name) {
  order_track_.reserve(1024 * 2);
}

/**
 * @brief Create and send a new order to td engine
 */
int ITDHelper::sendOrder(const char* instr, double price, int vol, int dir,
                         int off, int acc_id, int stgid, uint32_t instr_hash) {
  order_track_.emplace_back();
  auto& track = order_track_.back();
  track.status = ODS(SEND);  // initial order state
  track.instr_hash = instr_hash;
  strncpy(track.instr, instr, sizeof(track.instr) - 1);
  track.price = price;
  track.vol = vol;
  track.dir = dir;
  track.off = off;
  track.stg_id = stgid;   // strategy id?
  track.acc_id = acc_id;  // account hash id

  int track_id = order_track_.size() - 1;
  track.local_id = track_id;  // index of local track record
  doSendOrder(track_id);
  return track_id;
}

int ITDHelper::deleteOrder(int track_id) {
  if (track_id < order_track_.size()) {
    auto& ot = order_track_[track_id];
    int status = ot.status;
    if (ot.order_ref >= 0 &&
        (status &
         (ODS(TDSEND) | ODS(EXECUTION) | ODS(ACCEPT) | ODS(MARKET_ACCEPT))) &&
        not(status & ODS(CLOSED))) {
      doDelOrder(track_id);
      ot.status |= ODS(CXLING);
      return 0;
    }
  }
  return -1;
}

const tRtnMsg* ITDHelper::getRtn() {
  const tRtnMsg* p = doGetRtn();
  if (p)  // update order track
  {
    uint32_t id = p->local_id;
    if (id < order_track_.size()) {
      auto& track = order_track_[id];
      track.status |= p->msg_type;  // update order status
      track.order_ref = p->order_ref;
      track.front_id = p->front_id;
      track.session_id = p->session_id;

      if (ODS(EXECUTION) == p->msg_type) {
        track.vol_traded += p->vol;  // update trading data
        track.amount_traded += p->vol * p->price;
        if (track.vol_traded >= track.vol)
          track.status |= ODS(CLOSED);  // close the order if all traded
      }
      return p;
    }
  }
  return nullptr;
}

void ITDHelper::closeOrderTrack() {
  //	for(auto &i : order_track_)
  //	{
  //		i.status |= ODS(CLOSED);
  //	}
  order_track_.clear();
}
