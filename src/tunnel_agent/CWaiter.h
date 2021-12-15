/*
 * CWaiter.h
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#ifndef SRC_TA_CWAITER_H_
#define SRC_TA_CWAITER_H_

#include <memory>

#include "CCommanderBase.h"
#include "CCrypt.h"
#include "ev.h"
using namespace std;

class CWaiter {
  enum {
    INIT,
    SSL_AUTH,
    GET_DES_KEY,
    DO_COMMAND,
    FINISH,
  };

 public:
  CWaiter(int fd);
  virtual ~CWaiter();

  // Callback of libev's io watcher, must be static
  // Dispatch to process for real work
  static void readWrite_cb(EV_P_ ev_io *w, int events);

  // The actual process of the read/write event.
  int process(EV_P_ ev_io *w, int events);

  // write as much data to the socket output buffer as possible
  // non-blocking
  bool write_cb();

  // read a full msg from the socket input buffer
  // blocking
  int read_cb();

  void resetListen(int events);

  void send(string &s);

  void desSendData(string &s);

  void sayFinish(uint8_t type = TYPE_FINISH_SUCC);

  void handleConnectionBroken();

  int getFd() { return fd_; }

 public:
  int init();

  int processPkg();

  int processSSLAuth(string &pkg);

  int get3DESkey(string &pkg);

  int doCommand(string &pkg);

 private:
  int fd_;     // socket managed by the waiter
  int state_;  // state machine code of the waiter

  uint32_t read_cur_;   // current position of the read buffer, if = size, then
                        // read complete
  uint32_t write_cur_;  // current position of the write buffer, if = size, then
                        // write complete

  bool is_write_open_;  // write event watched or not

  string read_buf_;   // buffer storing data from client
  string write_buf_;  // buffer of data to be sent to client

 private:
  // Keep the 3DES from client and encrypt/descrypt all msgs from/to client
  // Authentication and certifiaction with client use RSA, and use temporary
  // crypter. This one is dedicated for 3DES encryption/descryption.
  CCrypt crypter_;

  // Executor to run the commands from client on the host
  unique_ptr<CCommanderBase> p_commander_;
};

/**
 * @struct tWaiterWatcher
 * Combine the CWaiter with its corresponding libev io watcher.
 * Non-static data members are initialized by their order of declaration, thus
 * the watcher is guaranteed to exist before the initialization of the waiter.
 */
struct tWaiterWatcher {
  ev_io watcher;
  CWaiter waiter;

  tWaiterWatcher(int fd) : waiter(fd) {}

  // If CWaiter is only created through tWaiterWatcher, then the watcher's
  // address is always valid
  static ev_io *getWatcher(CWaiter *p_waiter) {
    return (ev_io *)((const char *)p_waiter - sizeof(ev_io));
  }
};

#endif /* SRC_TA_CWAITER_H_ */
