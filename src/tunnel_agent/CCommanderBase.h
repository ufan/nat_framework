/*
 * CCommanderBase.h
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 *
 * Base class for implementing the executable command from clients.
 * Each client is managed by one CWaiter, which is also the owner of the command
 * to be executed.
 */

#ifndef SRC_TA_CCOMMANDERBASE_H_
#define SRC_TA_CCOMMANDERBASE_H_

#include <string>

#include "protocol.h"
using namespace std;

class CWaiter;

class CCommanderBase {
 public:
  CCommanderBase(CWaiter *p_owner);
  virtual ~CCommanderBase();

  virtual int run(string &pkg) { return STATE_ABORT; }

  // send content back to client using p_owner_
  void sendToClient(string &content);

 protected:
  CWaiter *p_owner_;  // for communicate with the client
};

#endif /* SRC_TA_CCOMMANDERBASE_H_ */
