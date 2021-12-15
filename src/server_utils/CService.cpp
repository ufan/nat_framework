/*
 * CService.cpp
 *
 *  Created on: 2018年4月13日
 *      Author: sky
 */

#include "CService.h"

CService::CService(uint32_t rbuf_size, uint32_t wbuf_size)
    : head_size_(0),
      pkg_size_(0),
      state_(READ_HEAD),
      read_buf_(rbuf_size),
      write_buf_(wbuf_size) {
  timer_.data = this;
}

CService::~CService() {}

int CService::processHead(const char *p, uint32_t len) { return -1; }

int CService::processPkg(const char *p, uint32_t len) { return -1; }

void CService::onErr(int err) {
  LOG_TRACE("socket err, fd: %d, err: %s", fd_, strerror(err));
}

void CService::onConnectErr(int err) {
  LOG_TRACE("connect err, fd: %d, err: %s", fd_, strerror(err));
  close();
}

void CService::close() {
  CSocket::close();
  state_ = READ_HEAD;
  read_buf_.clear();
  write_buf_.clear();
}

int CService::sendData(const char *p, uint32_t len) {
  // no check of the failed state of cycleWrite, may be a bug (TODO)
  if (len && write_buf_.cycleWrite(p, len)) {
    addEvent(EV_WRITE);
    return len;
  }
  return -1;
}

bool CService::setHeadSize(uint32_t size) {
  if (size <= read_buf_.size_) {
    head_size_ = size;
    return true;
  }
  LOG_ERR("cannot set head size bigger than read buffer size.");
  return false;
}

bool CService::setPkgSize(uint32_t size) {
  if (size <= read_buf_.size_) {
    pkg_size_ = size;
    return true;
  }
  LOG_ERR("cannot set pkg size bigger than read buffer size.");
  return false;
}

// bug?? TODO
void CService::read_cb(EV_P_ ev_io *w, int events) {
  uint32_t len = READ_HEAD == state_ ? head_size_ : pkg_size_;
  while (read_buf_.tail_ < len) {
    int ret = read(fd_, (void *)(read_buf_.p_ + read_buf_.tail_),
                   len - read_buf_.tail_);
    if (ret > 0) {
      read_buf_.tail_ += ret;
      if (read_buf_.tail_ == len) {
        if (READ_HEAD == state_) {
          if (processHead(read_buf_.p_, read_buf_.tail_) < 0)  // err
          {
            close();
            return;
          }
          state_ = READ_BODY;
          len = pkg_size_;
        }

        if (read_buf_.tail_ >= len && READ_BODY == state_) {
          if (processPkg(read_buf_.p_, read_buf_.tail_) < 0)  // err
          {
            close();
            return;
          } else {
            state_ = READ_HEAD;
            read_buf_.tail_ = 0;
            len = head_size_;
          }
        }
      }
    } else if (ret == 0) {
      onErr(ECONNRESET);
      close();
      return;
    } else {
      if (errno != EAGAIN) {
        onErr(errno);
        close();
      }
      return;
    }
  }
}

/**
 * @brief Block sending data in the write_buf_
 */
void CService::write_cb(EV_P_ ev_io *w, int events) {
  if (write_buf_.len() > 0) {
    uint32_t len = 0;
    const char *p = write_buf_.cycleRead(len);
    do {
      int ret = write(fd_, p, len);
      if (ret > 0) {
        write_buf_.commitRead(ret);
        p = write_buf_.cycleRead(len);
      } else if (ret == 0) {
        onErr(ECONNRESET);
        close();
        return;
      } else {
        if (errno != EAGAIN) {
          onErr(errno);
          close();
        }
        return;
      }
    } while (len > 0);
  }
  stop(EV_WRITE);
}

void CService::connect_cb(EV_P_ ev_io *w, int events) {
  ev_timer_stop(EV_DEFAULT, &timer_);  // stop timer watcher
  stop();                              // stop io watcher

  int err = 0;
  socklen_t len = sizeof(err);
  if (0 > getsockopt(w->fd, SOL_SOCKET, SO_ERROR, &err, &len)) {
    int e = errno;
    LOG_ERR("getsockopt %d err:%s", w->fd, strerror(e));
    return onConnectErr(e);
  }
  if (err) {
    onConnectErr(err);
  } else {
    onConnected();
  }
}

void CService::connect_timeout_cb(EV_P_ ev_timer *w, int events) {
  stop();  // stop io watcher only, timer watcher is one-shot, so no need to
           // close
  onConnectErr(ETIMEDOUT);
}

static void ev_connect_cb(EV_P_ ev_io *w, int events) {
  (static_cast<CService *>(w->data))->connect_cb(EV_A_ w, events);
}

static void ev_connect_timeout_cb(EV_P_ ev_timer *w, int events) {
  (static_cast<CService *>(w->data))->connect_timeout_cb(EV_A_ w, events);
}

/**
 * @brief Summary
 * @param[in] ip
 * @param[in] port
 * @param[in]
 * @return Description
 */
bool CService::connect(string ip, uint16_t port, ev_tstamp timeout) {
  // Create a new socket, timeout > 0 for non-blocking socket
  // If in non-blocking mode, timer watcher with the timeout parameter will be
  // added to the loop
  if (fd_ < 0) {
    int type = timeout > 0. ? SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC
                            : SOCK_STREAM | SOCK_CLOEXEC;
    if (-1 == (fd_ = socket(AF_INET, type, 0))) {
      LOG_ERR("error in create socket:%s", strerror(errno));
      return false;
    }
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_aton(ip.c_str(), &addr.sin_addr);

  // For non-blocking mode, connect will return -1 and EINPROGRESS immediately
  // if connection is not established immediately. This is not a connection
  // failure, since connection may cost some time to establish. For blocking
  // mode, returning -1 means failure.
  if (-1 == ::connect(fd_, (struct sockaddr *)&addr, sizeof(struct sockaddr))) {
    // connect trial failed no matter blocking or non-blocking
    if (errno != EINPROGRESS) {
      int e = errno;
      LOG_ERR("connect error:%s", strerror(e));
      ::close(fd_);
      fd_ = -1;
      return false;
    }

    // errno == EINPROGRESS only occurs in non-blocking mode
    // start io watcher reset callback to wait for connection success
    start(EV_WRITE);                 // default callback is read_write_cb
    ev_set_cb(&io_, ev_connect_cb);  // reset to connect_cb

    // add timer watcher in non-blocking mode, so that the connect trial will
    // not last forever
    if (timeout > 0.) {
      ev_timer_init(&timer_, ev_connect_timeout_cb, timeout, 0.);
      ev_timer_start(EV_DEFAULT, &timer_);
    }
  } else
    onConnected();

  return true;
}
