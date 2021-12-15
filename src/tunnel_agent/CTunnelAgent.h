/*
 * CTunnelAgent.h
 *
 *  Created on: 2017年9月26日
 *      Author: hongxu
 *
 * Comment by Yong:
 *
 * CTunnelAgent is used as a singleton. It is ran as a server and maintains a
 * libev event loop inside for accepting remote connections from a client. For
 * each connected client, a dedicated pair of ev_io watcher and CWaiter is
 * created. The watcher is then added to the event loop, so that messages from
 * the client can be processed by the corresponding CWaiter.
 *
 * Callback member functions of libev are defined as static. Since libev is a C
 * library, 'this' pointer is not recognized by libev.
 */

#ifndef SRC_TA_CTUNNELAGENT_H_
#define SRC_TA_CTUNNELAGENT_H_

#include <map>
#include <memory>
#include <string>

#include "CConfig.h"
#include "CWaiter.h"
#include "Logger.h"
#include "compiler.h"
#include "ev.h"
using namespace std;

class CTunnelAgent {
  CTunnelAgent();

 public:
  virtual ~CTunnelAgent();

  bool init(string cfg_path);

  void run();

  void releaseWaiter(int fd);

  static void accept_cb(EV_P_ ev_io* w, int events);

  static void terminate_cb(EV_P_ ev_signal* w, int events);

  static CConfig* getConfig();

  static CTunnelAgent* instance();

 private:
  int listen_fd_;
  map<int, unique_ptr<tWaiterWatcher> > waiters_;
};

#endif /* SRC_TA_CTUNNELAGENT_H_ */
