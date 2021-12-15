/*
 * CTunnelAgent.cpp
 *
 *  Created on: 2017年9月26日
 *      Author: hongxu
 */

#include "CTunnelAgent.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "CStrategyManager.h"

CConfig *CTunnelAgent::getConfig() {
  static CConfig s_config;
  return &s_config;
}

CTunnelAgent::CTunnelAgent() : listen_fd_(-1) {}

CTunnelAgent::~CTunnelAgent() {}

bool CTunnelAgent::init(string cfg_path) {
  // init configure class
  CConfig *p_config = getConfig();
  ASSERT_RET(p_config->init(cfg_path), false);

  // init logger
  string logcfg = p_config->getVal<string>("COMMON", "log_conf");
  ASSERT_RET(initLogger(logcfg), false);

  // init listen socket
  string ip = p_config->getVal<string>("AGENT", "ip");
  uint16_t port = p_config->getVal<uint16_t>("AGENT", "port");

  if (-1 == (listen_fd_ = socket(
                 AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))) {
    LOG_ERR("create socket err: %s", strerror(errno));
    return false;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_aton(ip.c_str(), &addr.sin_addr);

  if (bind(listen_fd_, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    LOG_ERR("bind socket error: %s", strerror(errno));
    return false;
  }

  if (listen(listen_fd_, 10) == -1) {
    LOG_ERR("listen socket error: %s", strerror(errno));
    return false;
  }

  // set sd reuseful
  int reuseaddr = 1;
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseaddr,
                 sizeof(reuseaddr)) != 0) {
    LOG_ERR("reuse addr err: %s", strerror(errno));
    return false;
  }

  // init strategy shm table
  ASSERT_RET(CStrategyManager::commInit(), false);

  return true;
}

void CTunnelAgent::run() {
  // start ev, what about other backends like poll and aio
  struct ev_loop *loop = ev_default_loop(EVBACKEND_EPOLL | EVFLAG_NOENV);

  struct ev_io listen_wather;  // listen for new client connection
  ev_io_init(&listen_wather, accept_cb, listen_fd_, EV_READ);
  ev_io_start(loop, &listen_wather);

  ev_signal exitsig;
  ev_signal_init(&exitsig, terminate_cb, SIGTERM);
  ev_signal_start(loop, &exitsig);
  ev_unref(loop);  // this watcher should not keep the loop running when no
                   // other watcher active

  ev_signal intsig;
  ev_signal_init(&intsig, terminate_cb, SIGINT);
  ev_signal_start(loop, &intsig);
  ev_unref(loop);

  LOG_INFO("agent start.");
  ev_run(loop, 0);  // loop forever, so the above stack variables is safe to use
                    // until exist.

  LOG_INFO("END RUNNING.");
}

void CTunnelAgent::accept_cb(EV_P_ ev_io *w, int events) {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  if (unlikely(EV_ERROR & events)) {
    LOG_ERR("listen socket got an ev_error event.");
    ev_break(ev_default_loop(0), EVBREAK_ALL);
    return;
  }

  int fd =
      accept4(CTunnelAgent::instance()->listen_fd_, (struct sockaddr *)&addr,
              &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (fd < 0) {
    LOG_ERR("accept err: %s", strerror(errno));
    return;
  }

  LOG_INFO("%d|accept a client.", fd);

  unique_ptr<tWaiterWatcher> p_waiter(new tWaiterWatcher(fd));

  // libev is c library, do not recognize the implicit 'this' pointer
  instance()->waiters_[fd] = move(p_waiter);
}

void CTunnelAgent::releaseWaiter(int fd) {
  map<int, unique_ptr<tWaiterWatcher> >::iterator iter = waiters_.find(fd);
  if (iter == waiters_.end()) {
    LOG_ERR("%d|memory leak!!!", fd);
    return;
  }

  LOG_INFO("%d|erase waiter.", fd);
  waiters_.erase(iter);
}

void CTunnelAgent::terminate_cb(EV_P_ ev_signal *w, int events) {
  close(CTunnelAgent::instance()->listen_fd_);
  ev_break(ev_default_loop(0), EVBREAK_ALL);
  LOG_INFO("got terminate signal. process exits.");
  exit(0);
}

CTunnelAgent *CTunnelAgent::instance() {
  static CTunnelAgent agent;
  return &agent;
}
