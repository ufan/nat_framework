/*
 * CSignalRequester.cpp
 *
 *  Created on: Jun 26, 2018
 *      Author: hongxu
 *
 * Comment by Yong:
 * As a client.
 *
 */

#include "CSignalRequester.h"

#include "SignalAgentProtocol.h"

extern long g_system_time_ns;

CSignalRequester::CSignalRequester(long timeout, uint32_t buf_size)
    : CService(buf_size, 4096), timeout_(timeout) {
  setHeadSize(sizeof(tSACmdHead));
}

CSignalRequester::~CSignalRequester() {}

int CSignalRequester::processHead(const char *p, uint32_t len) {
  tSACmdHead *p_head = (tSACmdHead *)p;
  if (!CHECK_SA_HEAD(p_head) || !setPkgSize(p_head->len)) {
    LOG_INFO("bad data");
    return -1;
  }
  return 0;
}

/**
 * @brief Receive new frame data from server and write to local page files
 */
int CSignalRequester::processPkg(const char *p, uint32_t len) {
  // update last alive time
  lst_alive_time_ = g_system_time_ns;

  tSACmdHead *p_head = (tSACmdHead *)p;
  switch (p_head->cmd) {
    case SACMD_SIGNAL_DATA: {  // new frame received
      auto &w = writers_[((tSASignalData *)p)->sig_idx];
      w->write(((tSASignalData *)p)->sig_data, len - sizeof(tSASignalData));
      break;
    }
    case SACMD_REP_HEATBEAT:  // heart beat response
      break;
    default:
      LOG_INFO("unknown cmd %d", p_head->cmd);
      return -1;
  }
  return 0;
}

// send the request to server for signal data
// only did once upon connection accepted, then just wait for new data
// and send the heart beat msg.
void CSignalRequester::sendRequest() {
  tSARequestSignal head;
  head.cmd = SACMD_REQ_SIG;
  head.sig_cnt = signames_.size();
  head.len = sizeof(head);
  for (auto &i : signames_) {
    head.len += i.size() + 1;
  }
  sendData((const char *)&head, sizeof(head));
  for (auto &i : signames_) {
    sendData(i.c_str(), i.size() + 1);
  }
}

/**
 * @brief Upon successful connection, immediately start the io watcher in the
 * event loop and watch the read event on its socket.
 */
int CSignalRequester::onConnected() {
  is_connect_ = true;

  sendRequest();

  addEvent(EV_READ);

  lst_alive_time_ = g_system_time_ns;
  LOG_INFO("remote %s:%d connected.", ip_.c_str(), port_);
  return 0;
}

void CSignalRequester::timeout(EV_P_ ev_timer *w, int events) {
  ev_timer_stop(EV_DEFAULT, &timer_);
  connect(ip_, port_, timeout_ / 1000000000L);
}

static void ev_timeout_cb(EV_P_ ev_timer *w, int events) {
  (static_cast<CSignalRequester *>(w->data))->timeout(EV_A_ w, events);
}

void CSignalRequester::onConnectErr(int err) {
  is_connect_ = false;
  LOG_INFO("%s:%d connect err, fd: %d, err: %s", ip_.c_str(), port_, fd_,
           strerror(err));
  CService::close();
  if (err == ETIMEDOUT) {
    connect(ip_, port_, timeout_ / 1000000000L);
  } else {
    ev_timer_init(&timer_, ev_timeout_cb, timeout_ / 1000000000L, 0.);
    ev_timer_start(EV_DEFAULT, &timer_);
  }
}

void CSignalRequester::close() {
  is_connect_ = false;
  CService::close();
  connect(ip_, port_, timeout_ / 1000000000L);
}

void CSignalRequester::tryHeartBeat() {
  if (is_connect_ &&
      (lst_heartbeat_time_ + (timeout_ >> 1) < g_system_time_ns)) {
    tSACmdHead head;
    head.cmd = SACMD_REQ_HEATBEAT;
    sendData((const char *)&head, sizeof(head));
    lst_heartbeat_time_ = g_system_time_ns;
  }
}

bool CSignalRequester::init(const json &j_conf) {
  // get server ip and port
  ip_ = j_conf["remote_ip"];
  port_ = j_conf["remote_port"];

  // get local prefix of io page file name
  string local_prefix;
  if (j_conf.find("local_prefix") != j_conf.end()) {
    local_prefix = j_conf["local_prefix"];
  }

  // init writers for the requested signal frames
  for (auto &s : j_conf["signals"]) {
    string name = s;
    unique_ptr<CSignalWriter> w(new CSignalWriter(local_prefix + name));
    if (w->hasLoad()) {
      writers_.push_back(move(w));
    } else {
      LOG_ERR("%s signal writer init failed.", name.c_str());
      return false;
    }
    signames_.push_back(name);
  }

  // try to connect the server
  connect(ip_, port_, timeout_ / 1000000000L);
  return true;
}

bool CSignalRequester::testOK() {
  if (is_connect_ && (lst_alive_time_ + timeout_ > g_system_time_ns)) {
    return true;
  } else if (is_connect_) {
    is_connect_ = false;
    CService::close();
    connect(ip_, port_, timeout_ / 1000000000L);
    LOG_INFO("%s:%d heart beat timeout, reconnecting...", ip_.c_str(), port_);
  }
  return false;
}
