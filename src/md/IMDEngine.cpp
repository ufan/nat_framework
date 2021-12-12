/*
 * IMDEngine.cpp
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#include "IMDEngine.h"

#include <time.h>

#include <memory>

#include "ATSysUtils.h"
#include "CSystemIO.h"
#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "IOCommon.h"
#include "MurmurHash2.h"

IMDEngine::IMDEngine() {
  long begin = CTimer::instance().getDayBeginTime();
  long now = time(NULL);
  if (begin + 6 * 3600 <= now && now <= begin + 18 * 3600)  // 6:00 ~ 18:00
  {
    day_night_mode_ = MODE_DAY;
  } else
    day_night_mode_ = MODE_NIGHT;
}

IMDEngine::~IMDEngine() {}

bool IMDEngine::initEngine(const json &j_conf) {
  // Init the engine (each type of front require different procedure)
  config_ = j_conf;
  if (!init()) {
    ALERT("init engine err.");
    return false;
  }

  // Configure and initialize md io writer
  // create md io directory using the engine name as subdirectory
  string write_path = string(IO_MDENGINE_BASE_PATH) + name_;
  if (!createPath(write_path)) {
    ALERT("create md_io directory %s failed.", write_path.c_str());
    return false;
  }

  // default page size: 128 MB
  if (j_conf["MDEngine"].find("PageSize") != j_conf["MDEngine"].end()) {
    long page_size = j_conf["/MDEngine/PageSize"_json_pointer];
    page_size *= MB;
    md_writer_.setPageSize(page_size);
  }

  // load the latest page
  if (!md_writer_.init(write_path + "/md")) {
    ALERT("init %s md writer err.", name_.c_str());
    return false;
  }

  // Generate hash id from engine name
  self_id_ = (int)HASH_STR(name().c_str());

  // Initialize system io
  if (!CSystemIO::instance().init()) {
    ALERT("init system io writer err.");
    return false;
  }

  // Get a copy of the base information of all listed instruments from the
  // specified td engine at this stage, init base info reqo and write base info
  // to md io.
  // Query for base information to this engine will return this copy from base
  // info repo.
  // But, initially, only TD has the base info, which is from the trade front
  if (!getBaseInfo(j_conf["/TDEngine/engine_name"_json_pointer],
                   j_conf["/TDEngine/timeout"_json_pointer])) {
    ALERT("getBaseInfo failed.");
    return false;
  }
  return true;
}

// Write 'start md' command to md io
void IMDEngine::writeStartSignal() {
  int cmd = IO_MD_START;
  md_writer_.write(&cmd, sizeof(cmd));
}

void IMDEngine::runEngine() {
  ENGLOG("md_engine start...");

  // Start md engine thread and try to connect and login to the front
  if (!start()) {
    release();
    ALERT("engine start failed.");
    return;
  }

  // Subscribe to pre-defined list of instruments in json configuration
  auto &j = config_["MDEngine"];
  if (j.find("subscribe") != j.end()) {
    engine_subscribe(j["subscribe"]);
  }

  ENGLOG("md_engine start listening.");

  // Start listen on system io
  unique_ptr<CRawIOReader> sys_reader(CSystemIO::instance().createReader());
  writeStartSignal();  // write md_start message to md io

  // Enter the event loop
  // md engine only fetch the new message from system io
  do_running_ = true;
  while (do_running_) {
    uint32_t len;
    tSysIOHead *p = (tSysIOHead *)sys_reader->read(len);
    if (p) {
      if (p->to == self_id_) {  // only respond message sent to this engine
        switch (p->cmd) {
          case IO_HEAT_BEAT: {  // client can use heart beat to check whether md
                                // engine is still alive
            tSysIOHead ack = {IO_HEAT_BEAT_ACK, p->source, self_id_,
                              p->back_word};
            CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
            LOG_DBG("reply heatbeat to %d", p->source);
            break;
          }
          case IO_QUERY_SUBS_INSTR: {  // return the list of subscribed
                                       // instruments
            tSysIOHead ack = {IO_RSP_QUERY_SUBS_INSTR, p->source, self_id_,
                              p->back_word};
            string data((const char *)&ack, sizeof(ack));
            json j_vec(querySubscribedInstrument());
            data += j_vec.dump();
            CSystemIO::instance().getWriter().write(data.c_str(),
                                                    data.size() + 1);
            break;
          }
          case IO_SUBS_INSTR: {  // subscribe to new instruments
            try {
              auto j = json::parse(p->data);
              vector<string> subs = j;
              engine_subscribe(subs);
              tSysIOHead ack = {IO_ACK, p->source, self_id_, p->back_word};
              CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
            } catch (...) {
              ALERT("json parse err: %s", p->data);
            }
            break;
          }
          case IO_UNSUBS_INSTR: {  // unsubscribe instruments
            try {
              auto j = json::parse(p->data);
              vector<string> unsubs = j;
              engine_unsubscribe(unsubs);
              tSysIOHead ack = {IO_ACK, p->source, self_id_, p->back_word};
              CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
            } catch (...) {
              ALERT("json parse err: %s", p->data);
            }
            break;
          }
          case IO_TD_REQ_BASE_INFO: {  // return a copy of all listed
                                       // instruments of this market day
            string data = CTradeBaseInfo::toSysIOStruct(
                ((tSysIOHead *)p)->source, self_id_,
                ((tSysIOHead *)p)->back_word);
            CSystemIO::instance().getWriter().write(data.data(), data.size());
            LOG_DBG("response base info query.");
            break;
          }
        }
      }
    } else {
      usleep(200000);  // 200ms
    }
  }

  release();
  // join();			// wait for finish complete

  ENGLOG("md_engine stopped.");
}

// Get the list of subscribed instruments of all users through this engine
vector<string> IMDEngine::querySubscribedInstrument() {
  vector<string> res;
  for (auto &kv : subs_instr_) {
    res.push_back(kv.first);
  }
  return res;
}

// Subscribe instruments:
// 1. If it's newly subscribed, push it to md engine for subsription
// 2. If it's already listed in subscribed collection, increment the usage
// counter by 1
void IMDEngine::engine_subscribe(const vector<string> &instr) {
  auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
  vector<string> real_subs;
  for (const auto &i : instr_set) {
    auto itr = subs_instr_.find(i);
    if (itr == subs_instr_.end()) {
      real_subs.push_back(i);
      subs_instr_[i] = 1;
    } else {
      itr->second++;
    }
  }
  subscribe(real_subs);
}

// Unsubscribe instruments:
// 1. If no user are using it, push it to md engine for unsubscribing
// 2. Else just decrement the usage counter by 1, keep subscription
void IMDEngine::engine_unsubscribe(const vector<string> &instr) {
  auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
  vector<string> real_unsubs;
  for (const auto &i : instr_set) {
    auto itr = subs_instr_.find(i);
    if (itr != subs_instr_.end()) {
      if (--(itr->second) <= 0) {
        real_unsubs.push_back(i);
        subs_instr_.erase(i);
      }
    }
  }
  unsubscribe(real_unsubs);
}

// Get base information from the specified td engine.
// Request is sent and respond is read using the system io.
// The retrieved data is used to initialize the trade base repository and
// written to the md io.
bool IMDEngine::getBaseInfo(string td_engine_name, int timeout) {
  int td_engine_id = (int)HASH_STR(td_engine_name.c_str());
  string res = sysRequest(IO_TD_REQ_BASE_INFO, IO_TD_RSP_BASE_INFO,
                          td_engine_id, self_id_, timeout);
  if (res.size()) {
    const tIOTDBaseInfo *p =
        (const tIOTDBaseInfo *)((const tSysIOHead *)res.data())->data;
    CTradeBaseInfo::set(p);
    md_writer_.write(res.data(), res.size());  // record this in md flow
    return true;
  }
  return false;
}
