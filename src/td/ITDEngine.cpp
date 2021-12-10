/*
 * ITDEngine.cpp
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#include "ITDEngine.h"

#include <fstream>

#include "CSystemIO.h"
#include "compiler.h"
#include "utils.h"

atomic_flag ITDEngine::flag_ = ATOMIC_FLAG_INIT;

ITDEngine::ITDEngine() {}

ITDEngine::~ITDEngine() {}

bool ITDEngine::initEngine(const json &j_conf) {
  /******************* Configure front related   *****************************/

  auto &j_range = j_conf["/TDEngine/request_id_range"_json_pointer];
  request_id_start_ = j_range[0];
  request_id_end_ = j_range[1];
  request_id_ = request_id_start_;

  // Step 1: init trader front, and MT-execution starts from here
  // - accounts collection init,
  // - front connection and subscription to public and private flow
  // - authentication, login, settlement confirmation for each account
  if (!init(j_conf)) {
    ALERT("init engine err.");
    return false;
  }
  LOG_DBG("init engine succeeded.");

  // Step 2: fetch instrument base data and init trade base repo
  // query using the first account: fetch instrument information
  // This is the first time trade base repo is filled.
  if (!getBaseInfo()) {
    ALERT("getBaseInfo err.");
    return false;
  }
  LOG_DBG("getBaseInfo successfully.");

  // Step 3: init order track mmap file storage, then fetch the exiting order
  // from front and fill into the track mmap file
  if (!loadOrderTrack() || !updateOrderTrack()) {
    ALERT("init order track err.");
    return false;
  }
  LOG_DBG("loadOrderTrack and updateOrderTrack successfully.");

  // Step 4: init risk management
  // TBU
  if (!initAccountUtilis(j_conf)) {
    ALERT("init account utilis err.");
    return false;
  }

  /******************* Step 4: Configure event-loop related (i.e., IO related)
   * *****************************/

  // Generate hash id of this engine from engine name (should be unique)
  // This id is the identifier for the nodes of communication.
  self_id_ = (int)HASH_STR(name().c_str());

  // Create td's io directory using the engine name
  // The engine name should be unique in the system.
  string io_dir = string(IO_TDENGINE_BASE_PATH) + name();
  if (!createPath(io_dir)) {
    ALERT("create td_io directory %s failed.", io_dir.c_str());
    return false;
  }

  // Init TD engine output  message channel
  // When initialized, it always load the latest frame for writing
  // This writer is MT-safe, but not MP-safe. Each engine maintains its own
  // exclusive message channel, with engine's name as page prefix.
  if (!writer_.init(io_dir + "/tdsend")) {
    ALERT("init writer err.");
    return false;
  }

  // Init system's io
  // when initialized, it always load the latest frame for writing
  if (!CSystemIO::instance().init()) {
    ALERT("init system io writer err.");
    return false;
  }

  // Add system io reader
  // The reader always points to the current latest frame of the current latest
  // system page.
  // From this point on, the engine will receive the incoming system message
  read_pool_.add(0, CSystemIO::instance().createReader());  // 0 for system io

  return true;
}

bool ITDEngine::initAccountUtilis(const json &j_conf) {
  int acc_cnt = getAccountCnt();

  // the information is from top-level 'Account'
  auto &acc_conf = j_conf["Account"];
  for (int i = 0; i < acc_cnt; ++i) {
    acc_utilis_.emplace_back(new RiskTop);
    auto &acc = acc_utilis_.back();

    // use the default config if no specific config exists
    auto conf = j_conf["AccountDefault"];
    if (i < acc_conf.size()) conf = acc_conf[i];
    string acc_name = name() + string(".sub") + to_string(i);
    if (!acc->init(acc_name.c_str(), conf)) {
      ALERT("init %d account position failed.", i);
      return false;
    }

    if (!acc->load()) {
      ALERT("load last account err.");
      return false;
    }
    for (int i = request_id_start_; i < request_id_; i++) {
      acc->onOrderTrack(&get_request_track(i));
    }

    for (auto &kv : CTradeBaseInfo::instr_info_) {
      if (not acc->regInstr(kv.second.instr)) {
        ALERT("account register instr %s err", kv.second.instr);
        return false;
      }
    }
  }
  return true;
}

// Init the tracked order mmap file using engine's name as file name
// The mmap file is opened for writing.
bool ITDEngine::loadOrderTrack() {
  // Load writable track order mmap file
  if (otmmap_.load(name(), true)) {
    // Get the memory buffer pointer of mmap file
    tOrderTrackMmap *p = otmmap_.getBuf();
    p->ver = 1;
    p->reserved = 0;
    request_track_ = p->order_track;
    return true;
  }
  return false;
}

inline void ITDEngine::engine_req_order_action(const tIOrderAction *data) {
  if ((uint32_t)(data->acc_idx) < acc_utilis_.size()) {
    req_order_action(data);
  } else {
    writeErrRtn(data, -1, "account idx err.");
  }
}

void ITDEngine::writeStartSignal() {
  int cmd = IO_TD_START;
  writer_.write(&cmd, sizeof(cmd));
}

void ITDEngine::engine_on_close() {
  for (auto &i : acc_utilis_) {
    i->save(CTradeBaseInfo::trading_day_, true);
  }
}

// Event loop for IO Page command processing
void ITDEngine::listening() {
  ENGLOG("start listening...");
  writeStartSignal();

  do_running_ = true;
  while (do_running_) {
    uint32_t len = 0;
    uint32_t ioid;  // hash_id_ of the Page IO type, 0 is system io

    // Fetch next un-processed cmd in reader pool
    // The default reader pool is SystemIO
    // TraderIO reader can be added by request from CTDHelperComm
    const char *p = read_pool_.seqRead(len, ioid);
    if (p) {
      if (0 != ioid)  // trader io message, this IO is added by CTDHelperComm
      {
        switch (*(int *)p) {
          case IO_SEND_ORDER: {
            req_order_insert((const tIOInputOrderField *)p);
            break;
          }
          case IO_ORDER_ACTION: {
            engine_req_order_action((const tIOrderAction *)p);
            break;
          }
        }
      } else if (((tSysIOHead *)p)->to == self_id_)  // system io message
      {
        tSysIOHead *p_head = (tSysIOHead *)p;
        switch (p_head->cmd) {
          case IO_TD_ADD_CLIENT: {  // add a reader for new client using hash_id
                                    // as identifier and write ack to system io
            read_pool_.add(p_head->source, p_head->data, -1, -1);
            tSysIOHead ack = {IO_TD_ACK_ADD_CLIENT, p_head->source, self_id_,
                              p_head->back_word};
            CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
            LOG_DBG("add client %d:%s, reader_cnt:%d", p_head->source,
                    p_head->data, read_pool_.size());
            break;
          }
          case IO_TD_REMOVE_CLIENT: {  // remove the reader for the client using
                                       // hash_id and no ack
            read_pool_.erase(p_head->source);
            LOG_DBG("remove client %d", p_head->source);
            break;
          }
          case IO_TD_QUIT: {
            do_running_ = false;  // quit the engine
            break;
          }
          case IO_TD_REQ_BASE_INFO: {  // write a copy of base info to system io
            string data = CTradeBaseInfo::toSysIOStruct(
                p_head->source, self_id_, p_head->back_word);
            CSystemIO::instance().getWriter().write(data.data(), data.size());
            LOG_DBG("response base info query.");
            break;
          }
          case IO_TD_REQ_ORDER_TRACK: {  // write back a list of order
                                         // corresponding to system io, the
                                         // orders belong to the message source
            rspOrderTrack(p_head);
            break;
          }

          case IO_USER_ADD_EXEC_ORDER: {
            tIOUserAddExecOrder *pcmd = (tIOUserAddExecOrder *)p;
            auto &util = acc_utilis_[pcmd->acc_idx];
            util->onNew(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol,
                        pcmd->instr_hash, -1);
            if (auto pmod = util->getModInstr(pcmd->instr_hash)) {
              pmod->onTrd(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol,
                          pcmd->vol, pcmd->price, pcmd->vol);
            }
            break;
          }
        }
      }
    }
  }

  engine_on_close();
  ENGLOG("listen stopped.");
}

void ITDEngine::writeErrRtn(const tIOInputOrderField *data, int errid,
                            const char *msg, int msgtp) {
  tIOrderRtn *p = (tIOrderRtn *)writer_.prefetch(sizeof(tIOrderRtn));
  p->cmd = IO_ORDER_RTN;
  p->to = data->from;
  p->rtn_msg.msg_type = msgtp;
  p->rtn_msg.local_id = data->local_id;
  p->rtn_msg.instr_hash = data->instr_hash;
  memcpy(p->rtn_msg.instr, data->instr, sizeof(p->rtn_msg.instr));
  p->rtn_msg.price = data->price;
  p->rtn_msg.vol = data->vol;
  p->rtn_msg.dir = data->dir;
  p->rtn_msg.off = data->off;
  p->rtn_msg.order_ref = -1;
  p->rtn_msg.front_id = -1;
  p->rtn_msg.session_id = -1;
  p->rtn_msg.errid = errid;
  strncpy(p->rtn_msg.msg, msg, sizeof(p->rtn_msg.msg));
  p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
  writer_.commit();
}

void ITDEngine::writeErrRtn(const tIOrderAction *data, int errid,
                            const char *msg, int msgtp) {
  tIOrderRtn *p = (tIOrderRtn *)writer_.prefetch(sizeof(tIOrderRtn));
  p->cmd = IO_ORDER_RTN;
  p->to = data->from;
  p->rtn_msg.msg_type = msgtp;
  p->rtn_msg.local_id = data->local_id;
  p->rtn_msg.instr_hash = data->instr_hash;
  memcpy(p->rtn_msg.instr, data->instr, sizeof(p->rtn_msg.instr));
  p->rtn_msg.order_ref = data->order_ref;
  p->rtn_msg.front_id = data->front_id;
  p->rtn_msg.session_id = data->session_id;
  p->rtn_msg.errid = errid;
  strncpy(p->rtn_msg.msg, msg, sizeof(p->rtn_msg.msg));
  p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
  writer_.commit();
}

tIOrderRtn *ITDEngine::writeRtnFromTrack(tOrderTrack &request_track) {
  tIOrderRtn *p = (tIOrderRtn *)writer_.prefetch(sizeof(tIOrderRtn));
  p->cmd = IO_ORDER_RTN;
  p->to = request_track.from;
  p->rtn_msg.local_id = request_track.local_id;
  strncpy(p->rtn_msg.instr, request_track.instr, sizeof(p->rtn_msg.instr) - 1);
  p->rtn_msg.instr[sizeof(p->rtn_msg.instr) - 1] = '\0';
  p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
  p->rtn_msg.price = request_track.price;
  p->rtn_msg.vol = request_track.vol;
  p->rtn_msg.dir = request_track.dir;
  p->rtn_msg.off = request_track.off;
  p->rtn_msg.order_ref = request_track.order_ref;
  p->rtn_msg.front_id = request_track.front_id;
  p->rtn_msg.session_id = request_track.session_id;
  return p;
}

void ITDEngine::rspOrderTrack(tSysIOHead *req) {
  string data(sizeof(tIOrderTrack), 0);
  int cnt = 0;
  for (int i = request_id_start_; i < request_id_; i++)  // start from 1
  {
    tOrderTrack &ot = get_request_track(i);
    if (ot.from == req->source) {
      data += string((const char *)&ot, sizeof(ot));
      cnt++;
    }
  }
  tIOrderTrack *head = (tIOrderTrack *)data.data();
  head->cmd = IO_TD_RSP_ORDER_TRACK;
  head->to = req->source;
  head->source = self_id_;
  head->back_word = req->back_word;
  head->cnt = cnt;
  CSystemIO::instance().getWriter().write(data.data(), data.size());
  LOG_DBG("response order track query to %d, order cnt:%d.", head->to,
          head->cnt);
}
