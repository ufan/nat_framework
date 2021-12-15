/*
 * CSignalServer.h
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 *
 * Act as a CSignalAgent server's sub-processing unit. It manages the
 * communication with one client of the CSignalAgent server.
 * It has a list of readers of signal ios.
 */

#ifndef SRC_SIGNAL_AGENT_CSIGNALSERVER_H_
#define SRC_SIGNAL_AGENT_CSIGNALSERVER_H_

#include <memory>
#include <set>
#include <vector>

#include "CService.h"
#include "CSignal.h"
using namespace std;

class CSignalServer : public CService {
 public:
  CSignalServer(int fd, long timeout, uint32_t buf_size = 4 * 4096);
  virtual ~CSignalServer() {}

  virtual int processHead(const char *p, uint32_t len);

  virtual int processPkg(const char *p, uint32_t len);

  void addReader(string sig_name);

  void tryRead(bool scan_unload = false);

  bool isOK();

 protected:
  // last time the server received a msg from client
  long lst_alive_time_ = 0;
  // 5s, the max interval that server keeps the connection alive without
  // receiving a msg from client
  long timeout_ = 5000000000L;

  vector<unique_ptr<CSignalReader>> readers_;  // list of signal readers
  set<string> read_signal_set_;                // set of signal io names
};

#endif /* SRC_SIGNAL_AGENT_CSIGNALSERVER_H_ */
