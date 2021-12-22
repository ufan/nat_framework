/*
 * CTDHelperPipe.cpp
 *
 *  Created on: Sep 7, 2018
 *      Author: hongxu
 */

#include "CTDHelperPipe.h"

#include "ATSysUtils.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "utils.h"

extern int g_replay_md_fake_wait_count;

bool CTDHelperPipe::init(const json& j_conf) {
  // 1. Create and run the child process with command "path args"
  //    Three pipes are established between parent and the child:
  //    a.) executor write fd -> child stdin
  //    b.) executor read fd  <- child stdout
  //    c.) executor error fd <- child stderr
  p_executor_ =
      CPipExecutorManager::instance().create(j_conf["path"], j_conf["args"]);
  if (not p_executor_) {
    ALERT("create pipe process failed.");
    return false;
  }
  setNonBlock(p_executor_->getErrFd());  // non block on err pipe

  // 2. calculate self id
  self_id_ = (int)HASH_STR(name_.c_str());

  // 3. update trade base info
  if (!CTradeBaseInfo::is_init_ && !qryTradeBaseInfo()) {
    ALERT("get td base information failed.");
    return false;
  }

  // 4. update order track
  if (!qryOrderTrack()) {
    ALERT("query order track failed,");
    return false;
  }
  return true;
}

void CTDHelperPipe::doSendOrder(int track_id) {
  tOrderTrack& order = getOrderTrack(track_id);
  json j;
  j["cmd"] = "sendOrder";
  j["from"] = self_id_;
  j["local_id"] = track_id;
  j["acc_idx"] = order.acc_id;
  j["instr_hash"] = order.instr_hash;
  j["instr"] = order.instr;
  j["price"] = order.price;
  j["vol"] = order.vol;
  j["dir"] = order.dir;
  j["off"] = order.off;
  j["stg_id"] = order.stg_id;
  writeJson(j);
}

void CTDHelperPipe::doDelOrder(int track_id) {
  tOrderTrack& order = getOrderTrack(track_id);
  json j;
  j["cmd"] = "delOrder";
  j["from"] = self_id_;
  j["local_id"] = track_id;
  j["acc_idx"] = order.acc_id;
  j["order_ref"] = order.order_ref;
  j["front_id"] = order.front_id;
  j["session_id"] = order.session_id;
  j["instr_hash"] = order.instr_hash;
  j["instr"] = order.instr;
  writeJson(j);
}

// Retreive as many pieces of return messages as possible from child
// Then, pop out the old message from the message queue
const tRtnMsg* CTDHelperPipe::doGetRtn() {
  if (p_executor_) {
    string data;
    char buf[4096];

    // Fetch all return messages currently available
    // Multiple pieces of message may be read into the same buffer
    int ret = ::read(p_executor_->getErrFd(), buf, sizeof(buf));
    if (ret > 0) {
      data.append(buf, ret);
      setBlock(p_executor_->getErrFd());
      while (data.back() != '\n') {
        ret = ::read(p_executor_->getErrFd(), buf, sizeof(buf));
        if (ret > 0)
          data.append(buf, ret);
        else
          break;
      }
      setNonBlock(p_executor_->getErrFd());

      // Separate the messages based '\n' indicator
      if (data.back() == '\n') {
        char* p = (char*)data.data();
        char* b = p;
        while (*b) {
          while (*p != '\n') p++;
          *p = '\0';  // indicates the end of this message before parsing
          json j = json::parse(string(b));
          b = ++p;

          rtn_msg_queue_.emplace();
          tRtnMsg& msg = rtn_msg_queue_.back();
          parseJsonStr(j, &msg);
          g_replay_md_fake_wait_count++;
        }
      }
    }
  }

  // Pop out the first return message in the queue (FIFO principle)
  if (not rtn_msg_queue_.empty()) {
    hold_result_ = rtn_msg_queue_.front();
    rtn_msg_queue_.pop();
    return &hold_result_;
  }

  return nullptr;
}

void CTDHelperPipe::release() {
  if (p_executor_) {
    CPipExecutorManager::instance().kill(p_executor_->getHash());
    p_executor_ = nullptr;
  }
}

bool CTDHelperPipe::qryTradeBaseInfo() {
  // send query request
  json j;
  j["cmd"] = "queryBaseInfo";
  writeJson(j);

  // wait for response and then update
  try {
    auto res = readJson();
    if (res.empty() || res["ret"].get<string>() != "OK") return false;

    json2BaseInfo(res);
  } catch (...) {
    ALERT("json parse err");
    return false;
  }
  return true;
}

bool CTDHelperPipe::qryOrderTrack() {
  // send query request
  json j;
  j["cmd"] = "qryOrderTrack";
  j["id"] = self_id_;
  writeJson(j);

  // wait for response and then update
  try {
    auto res = readJson();
    if (res.empty() || res["ret"].get<string>() != "OK") return false;

    const auto& ots = res["order_track"];
    for (const auto& i : ots) {
      tOrderTrack ot;
      ot.status = i["status"];
      ot.setInstrStr(i["instr"]);
      ot.instr_hash = INSTR_NAME_TO_HASH(ot.instr);
      ot.price = i["price"];
      ot.vol = i["vol"];
      ot.dir = i["dir"];
      ot.off = i["off"];
      ot.vol_traded = i["vol_traded"];
      ot.amount_traded = i["amount_traded"];
      ot.from = i["from"];
      ot.local_id = i["local_id"];
      ot.acc_id = i["acc_id"];
      ot.stg_id = i["stg_id"];
      ot.order_ref = i["order_ref"];
      ot.front_id = i["front_id"];
      ot.session_id = i["session_id"];
      order_track_.push_back(ot);
    }
  } catch (...) {
    ALERT("json parse err");
    return false;
  }
  return true;
}

// send single message to child process (block)
void CTDHelperPipe::writeJson(const json& j) {
  if (p_executor_) {
    string s = j.dump();
    write(p_executor_->getWriteFd(), s.c_str(), s.size());
    write(p_executor_->getWriteFd(), "\n", 1);
  }
}

// retreive single message from child process (block)
json CTDHelperPipe::readJson() {
  json res;
  if (p_executor_) {
    string data;
    char buf[4096];

    int ret = ::read(p_executor_->getReadFd(), buf, sizeof(buf));
    if (ret > 0)
      data.append(buf, ret);
    else  // 0 if the other end of pipe is closed, -1 if error occurs
      return res;

    while (data.back() != '\n') {
      ret = ::read(p_executor_->getReadFd(), buf, sizeof(buf));
      if (ret > 0)
        data.append(buf, ret);
      else
        break;
    }

    if (data.back() == '\n') {
      res = json::parse(data);
    }
  }
  return res;
}
