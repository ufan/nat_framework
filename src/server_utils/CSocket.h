/*
 * CSocket.h
 *
 *  Created on: 2018年4月13日
 *      Author: hongxu
 */

#ifndef SRC_MAPPORT_CSOCKET_H_
#define SRC_MAPPORT_CSOCKET_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "CBuffer.h"
#include "Logger.h"
#include "compiler.h"
#include "ev.h"

class CSocket {
 public:
  CSocket();
  virtual ~CSocket();

  virtual void close();

  virtual void read_write_cb(EV_P_ ev_io *w, int events);

  virtual void read_cb(EV_P_ ev_io *w, int events) = 0;

  virtual void write_cb(EV_P_ ev_io *w, int events) = 0;

  virtual void err_cb(EV_P_ ev_io *w, int events) {}

  void setFd(int fd) { fd_ = fd; }

  int getFd() { return fd_; }

 protected:
  int fd_;

  // ev_io relevant
 public:
  /**
   * @brief Stop watching (all events)
   */
  void stop() {
    if (is_watch_) {
      ev_io_stop(EV_DEFAULT, &io_);
      is_watch_ = false;
    }
  }

  /**
   * @brief Start watching the specified events (and only these events)
   */
  void start(int events) {
    stop();

    is_watch_ = true;
    ev_io_init(&io_, ev_read_write_cb, fd_, events);
    ev_io_start(EV_DEFAULT, &io_);
  }

  /**
   * @brief Stop watching the specified events
   */
  void stop(int events) {
    if (is_watch_)  // keep non-specified events if the watcher is running
    {
      int e = io_.events & ~events;
      if (e) {
        if (e != io_.events) {
          start(e);
        }
      } else  // if watcher not running, stop all events (just be definite)
        stop();
    }
  }

  /**
   * @brief Add new events to the watcher. Start the watcher if not yet.
   */
  void addEvent(int events) {
    int e = is_watch_ ? (io_.events | events) : events;
    if (e && (!is_watch_ || e != io_.events)) start(e);
  }

  /**
   * @brief Return the owner object of this watcher.
   * The owner will handle the watched events. The owner pointer is stored in
   * watcher's 'data' member.
   */
  static CSocket *getOwner(ev_io *w) { return static_cast<CSocket *>(w->data); }

  /**
   * @brief Callback of the watcher. Must be static since libev is C lib.
   * Just dispatch the processing to the real handler to which the watcher
   * belong.
   */
  static void ev_read_write_cb(EV_P_ ev_io *w, int events) {
    getOwner(w)->read_write_cb(EV_A_ w, events);
  }

 protected:
  ev_io io_;       // the watcher
  bool is_watch_;  // is watcher running? (or watching)
};

#endif /* SRC_MAPPORT_CSOCKET_H_ */
