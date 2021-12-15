/*
 * CListener.cpp
 *
 *  Created on: 2018年4月16日
 *      Author: hongxu
 */

#include "CListener.h"

CListener::CListener() {}

CListener::~CListener() {}

/**
 * @brief Create a non-blocking socket for listening
 */
bool CListener::open(string ip, uint16_t port) {
  if (-1 ==
      (fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))) {
    LOG_ERR("create socket err: %s", strerror(errno));
    return false;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_aton(ip.c_str(), &addr.sin_addr);

  if (bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    ::close(fd_);
    fd_ = -1;
    LOG_ERR("bind socket error: %s", strerror(errno));
    return false;
  }

  if (listen(fd_, 10) == -1) {
    ::close(fd_);
    fd_ = -1;
    LOG_ERR("listen socket error: %s", strerror(errno));
    return false;
  }

  // set sd reuseful
  int reuseaddr = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr,
                 sizeof(reuseaddr)) != 0) {
    ::close(fd_);
    fd_ = -1;
    LOG_ERR("reuse addr err: %s", strerror(errno));
    return false;
  }

  return true;
}

void CListener::read_cb(EV_P_ ev_io* w, int events) {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  int fd = accept4(w->fd, (struct sockaddr*)&addr, &addrlen,
                   SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (fd < 0) {
    LOG_ERR("accept err: %s", strerror(errno));
    return;
  }

  LOG_INFO("%d|accept remote client.", fd);

  onAccept(fd);
}

void CListener::err_cb(EV_P_ ev_io* w, int events) {
  LOG_ERR("listen socket got an error:%s.", strerror(errno));
  close();
  return;
}
