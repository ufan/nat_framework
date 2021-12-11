/*
 * CTDHelperComm.cpp
 *
 *  Created on: 2018年5月8日
 *      Author: hongxu
 */

#include "CTDHelperComm.h"

#include <time.h>

#include <memory>

#include "ATSysUtils.h"
#include "CSystemIO.h"
#include "CTradeBaseInfo.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "SysConf.h"
#include "utils.h"

extern long getCurMdTime();  // 获取当前的行情数据时间戳

bool CTDHelperComm::init(const json &j_conf) {
  return _init(j_conf["td_engine_name"], j_conf["timeout"]);
}

bool CTDHelperComm::_init(string engine, int timeout) {
  // step 1: init its own io page for w
  self_id_ = (int)HASH_STR(name_.c_str());
  tdsend_path_ = createTdSendPath(name_);
  if (tdsend_path_.empty()) {
    ALERT("td helper init failed when create tdsend path");
    return false;
  }
  if (!td_writer_.init(tdsend_path_)) {
    ALERT("init writer %s err.", tdsend_path_.c_str());
    return false;
  }

  // step 2: init the system io for r/w
  if (!CSystemIO::instance().init()) {
    ALERT("init system io writer err.");
    return false;
  }

  // step 3: ask td engine to add the reader to the helper's io page through
  // system io
  td_engine_id_ = (int)HASH_STR(engine.c_str());
  timeout_ = timeout;
  if (!notifyTDEngineAdd()) {
    ALERT("td_engine add client timeout.");
    return false;
  }

  // step 4: init a reader for td engine's io page (the latest frame)
  string td_read_path = string(IO_TDENGINE_BASE_PATH) + engine + "/tdsend";
  td_reader_.init(td_read_path, -1);

  // step 5: retrieve a copy of base info from td engine through system io
  //         a local base repo is established.
  if (!CTradeBaseInfo::is_init_ && !qryTradeBaseInfo()) {
    ALERT("get td base information failed.");
    return false;
  }

  // step 6: retrieve the list of track records of the orders from this helper
  // (identified by self_id_)
  if (!qryOrderTrack()) {
    ALERT("query order track failed,");
    return false;
  }
  return true;
}

/**
 * @details Request td engine to insert a new order through its own io
 */
void CTDHelperComm::doSendOrder(int track_id) {
  tOrderTrack &order = getOrderTrack(track_id);
  tIOInputOrderField *p =
      (tIOInputOrderField *)td_writer_.prefetch(sizeof(tIOInputOrderField));
  p->cmd = IO_SEND_ORDER;
  p->extra_nano = getCurMdTime();
  p->from = self_id_;  // from the name of this helper
  p->local_id = track_id;
  p->acc_idx = order.acc_id;
  p->instr_hash = order.instr_hash;
  memcpy(p->instr, order.instr, sizeof(p->instr));
  p->price = order.price;
  p->vol = order.vol;
  p->dir = order.dir;
  p->off = order.off;
  p->stg_id = order.stg_id;

  td_writer_.commit();
}

/**
 * @brief Request td engine to cancel an existing order through its own io
 */
void CTDHelperComm::doDelOrder(int track_id) {
  tOrderTrack &order = getOrderTrack(track_id);
  tIOrderAction *p =
      (tIOrderAction *)td_writer_.prefetch(sizeof(tIOrderAction));
  p->cmd = IO_ORDER_ACTION;
  p->from = self_id_;
  p->local_id = track_id;
  p->acc_idx = order.acc_id;
  p->order_ref = order.order_ref;
  p->front_id = order.front_id;
  p->session_id = order.session_id;
  p->instr_hash = order.instr_hash;
  memcpy(p->instr, order.instr, sizeof(p->instr));

  td_writer_.commit();
}

const tRtnMsg *CTDHelperComm::doGetRtn() {
  uint32_t len = 0;
  const tIOrderRtn *p = (const tIOrderRtn *)td_reader_.read(len);
  if (p) {
    if (p->cmd == IO_ORDER_RTN) {
      if (p->to == self_id_) return &(p->rtn_msg);
    } else if (p->cmd == IO_TD_START) {  // in case of td restart? TBU
      if (!notifyTDEngineAdd()) {
        ALERT("td_engine add client timeout.");
      }
    }
  }
  return nullptr;
}

string CTDHelperComm::createTdSendPath(string name) {
  string td_dir = string(IO_STRATEGY_BASE_PATH) + name;
  if (!createPath(td_dir)) {
    ALERT("create strategy dir %s err.", td_dir.c_str());
    return string();
  }
  return td_dir + "/tdsend";
}

bool CTDHelperComm::notifyTDEngineAdd() {
  return sysRequest(IO_TD_ADD_CLIENT, IO_TD_ACK_ADD_CLIENT, td_engine_id_,
                    self_id_, timeout_, tdsend_path_.c_str(),
                    tdsend_path_.size() + 1)
      .size();
}

void CTDHelperComm::notifyTDEngineRemove() {
  if (td_engine_id_ != 0 && self_id_ != 0) {
    tSysIOHead head = {IO_TD_REMOVE_CLIENT, td_engine_id_, self_id_, 0};
    CSystemIO::instance().getWriter().write((const void *)&head, sizeof(head));
    ENGLOG("remove td_helper client");
  }
}

bool CTDHelperComm::qryTradeBaseInfo() {
  string res = sysRequest(IO_TD_REQ_BASE_INFO, IO_TD_RSP_BASE_INFO,
                          td_engine_id_, self_id_, timeout_);
  if (res.size()) {
    const tIOTDBaseInfo *p =
        (const tIOTDBaseInfo *)((const tSysIOHead *)res.data())->data;
    CTradeBaseInfo::update(p);
    return true;
  }
  return false;
}

bool CTDHelperComm::qryOrderTrack() {
  string res = sysRequest(IO_TD_REQ_ORDER_TRACK, IO_TD_RSP_ORDER_TRACK,
                          td_engine_id_, self_id_, timeout_);
  if (res.size()) {
    const tIOrderTrack *p = (const tIOrderTrack *)res.data();
    order_track_.resize(p->cnt);
    for (int i = 0; i < p->cnt; i++) {
      order_track_[i] = p->track[i];
    }
    return true;
  }
  return false;
}
