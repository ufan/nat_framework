/*
 * CSignalAgent.cpp
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 */

#include "CSignalAgent.h"

#include "CTimer.h"

long g_system_time_ns = 0;

// Idle callback: just dispatch to read handler process()
static void ev_idle_cb(EV_P_ ev_idle *w, int revents) {
  CSignalAgent *agent = (CSignalAgent *)(w->data);

  // the agent only send new signal io frame to clients when the server is idle
  agent->process();
}

CSignalAgent::CSignalAgent() { idle_.data = this; }

CSignalAgent::~CSignalAgent() {}

/**
 * @brief Create a signal server for each newly connected client
 */
void CSignalAgent::onAccept(int fd) {
  char ip[20];
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  getpeername(fd, (struct sockaddr *)&addr, &addrlen);
  inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
  LOG_INFO("accept client from %s:%u", ip, htons(addr.sin_port));

  // create a signal server and start to process requests
  servers_.emplace_back(new CSignalServer(fd, timeout_, buf_size_));
  servers_.back()->start(EV_READ);
}

bool CSignalAgent::init(const json &j_conf) {
  // server listening ip and port
  ASSERT_RET(open(j_conf["listen_ip"], j_conf["listen_port"]), false);

  // buffer size for signal server's read_buf and signal requester's read_buf
  if (j_conf.find("buf_size") != j_conf.end()) buf_size_ = j_conf["buf_size"];

  timeout_ = j_conf["timeout_s"].get<long>() * 1000000000L;
  server_scan_period_ = j_conf["scan_period_us"].get<long>() * 1000;
  server_unload_scan_period_ =
      j_conf["unload_scan_period_ms"].get<long>() * 1000000L;

  for (auto &r : j_conf["listen_signals"]) {
    unique_ptr<CSignalRequester> req(new CSignalRequester(timeout_, buf_size_));
    ASSERT_RET(req->init(r), false);
    requesters_.push_back(move(req));
  }

  // start the idle watcher
  ev_idle_init(&idle_, ev_idle_cb);
  ev_idle_start(EV_DEFAULT, &idle_);

  //
  g_system_time_ns = CTimer::instance().getNano();

  return true;
}

void CSignalAgent::process() {
  // update system time
  g_system_time_ns = CTimer::instance().getNano();

  // check server's connected client is alive or not
  if (server_lst_scan_ + server_scan_period_ < g_system_time_ns) {
    server_lst_scan_ = g_system_time_ns;

    bool scan_unload = false;
    if (server_lst_scan_unload_ + server_unload_scan_period_ <
        g_system_time_ns) {
      scan_unload = true;
      server_lst_scan_unload_ = g_system_time_ns;
    }

    // read and send new frame, and disconnect the clients not alive
    for (int i = 0; i < servers_.size(); i++) {
      auto &s = servers_[i];
      if (!s->isOK()) {
        servers_.erase(servers_.begin() + i);
        i--;
        LOG_INFO("erased a server.");
        continue;
      }

      // send new data to clients
      s->tryRead(scan_unload);
    }
  }

  // keep requesters alive
  for (auto &r : requesters_) {
    r->testOK();
    r->tryHeartBeat();
  }

  long sleep_us = server_scan_period_ / 1000 / 2;
  if (sleep_us > 0) usleep(sleep_us);
}
