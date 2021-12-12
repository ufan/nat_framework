/*
 * CMDHelperComm.cpp
 *
 *  Created on: 2018年5月11日
 *      Author: hongxu
 */

#include "CMDHelperComm.h"

#include <memory>

#include "ATSysUtils.h"
#include "CSystemIO.h"
#include "CTradeBaseInfo.h"
#include "IOCommon.h"
#include "MurmurHash2.h"

bool CMDHelperComm::init(const json &j_conf) {
  string from;
  if (j_conf.find("from") != j_conf.end()) {
    from = j_conf["from"];
  }
  return _init(j_conf["md_engine_name"], j_conf["timeout"], from);
}

/**
 * @brief Initialize this helper
 * @param[in] engine Name of md engine to connect
 * @param[in] timeout Time out value for communicating with md engine
 * @param[in] from Timestamp string for representing reading all quotes from
 * this time. The format is like '20211212-15:26:05'. Default is the time when
 * this helper is initialized.
 * @return Description
 */
bool CMDHelperComm::_init(string engine, long timeout, string from) {
  // step 1: Init system io for sending request to md engine
  if (!CSystemIO::instance().init()) {
    ALERT("init system io writer err.");
    return false;
  }

  // setup ids for system io communication
  self_id_ = (int)HASH_STR(name_.c_str());
  md_engine_id_ = (int)HASH_STR(engine.c_str());
  timeout_ = timeout;

  // step 2: check if md engine is alive
  if (!testHeatBeat()) return false;

  // step 3: get a copy of the latest base info from md engine
  if (!qryTradeBaseInfo()) {
    ALERT("get base information failed.");
    return false;
  }

  // step 4: init the reader to md io for tick data
  string path = string(IO_MDENGINE_BASE_PATH) + engine + "/md";

  long start = -1;
  if (from.size()) {
    start = parseTime(from.c_str(), "%Y%m%d-%H:%M:%S");
  }

  return reader_.init(path, start);
}

/**
 * @brief Check if md engine is alive
 */
bool CMDHelperComm::testHeatBeat() {
  if (sysRequest(IO_HEAT_BEAT, IO_HEAT_BEAT_ACK, md_engine_id_, self_id_,
                 timeout_)
          .size())
    return true;
  ALERT("connect to md_engine timeout.");
  return false;
}

/**
 * @brief Get the list of instruments which the md engine has subscribed
 *        NOTE: multiple helper may connect to the same md engine, each
 *        subscribe to different set of instruments. Thus, the list of
 *        instruments subscribed by this helper may have less instruments than
 *        the md engine.
 */
vector<string> CMDHelperComm::getEngineSubscribedInstrument() {
  auto p = sysRequest(IO_QUERY_SUBS_INSTR, IO_RSP_QUERY_SUBS_INSTR,
                      md_engine_id_, self_id_, timeout_);
  if (p.size()) {
    try {
      auto j = json::parse(((tSysIOHead *)p.data())->data);
      vector<string> res = j;
      return res;
    } catch (...) {
      ALERT("json parse err: %s", ((tSysIOHead *)p.data())->data);
      return {};
    }
  }

  ALERT("query md_engine subscribed instrument timeout");
  return {};
}

bool CMDHelperComm::doSubscribe(const vector<string> &instr) {
  if (instr.size()) {
    json j(instr);
    string data = j.dump();
    auto p = sysRequest(IO_SUBS_INSTR, IO_ACK, md_engine_id_, self_id_,
                        timeout_, data.c_str(), data.size() + 1);
    if (p.empty()) {
      ALERT("subscribe instrument timeout");
      return false;
    }
  }
  return true;
}

bool CMDHelperComm::doUnsubscribe(const vector<string> &instr) {
  if (instr.size()) {
    json j(instr);
    string data = j.dump();
    auto p = sysRequest(IO_UNSUBS_INSTR, IO_ACK, md_engine_id_, self_id_,
                        timeout_, data.c_str(), data.size() + 1);
    if (p.empty()) {
      ALERT("unsubscribe instrument timeout");
      return false;
    }
  }
  return true;
}

/**
 * @brief Update local base info by fetching it from md engine
 * @param[out] update Flag indicating whether the update is carried out. False,
 * if it's the same trading day; True if not the same day.
 * @return Flag indicating whether query operation succeed.
 */
bool CMDHelperComm::qryTradeBaseInfo(bool &update) {
  string res = sysRequest(IO_TD_REQ_BASE_INFO, IO_TD_RSP_BASE_INFO,
                          md_engine_id_, self_id_, timeout_);
  if (res.size()) {
    const tIOTDBaseInfo *p =
        (const tIOTDBaseInfo *)((const tSysIOHead *)res.data())->data;
    bool ret = CTradeBaseInfo::update(p);
    if (&update != nullptr) update = ret;
    return true;
  }
  return false;
}

void CMDHelperComm::release() {
  IMDHelper::release();
  reader_.unload();
}
