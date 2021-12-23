/*
 * CLibLoader.h
 *
 *  Created on: 2017年10月4日
 *      Author: hongxu
 */

#ifndef SRC_TA_CEXECLOADER_H_
#define SRC_TA_CEXECLOADER_H_

#include <memory>

#include "CDirReciever.h"
#include "CFileReciever.h"
#include "ev.h"

class CExecLoader : public CFileReciever {
  struct tChildWatcher {
    ev_child wathcer;
    string name;
  };

  struct tDelWatcher {
    ev_timer timer;
    string file;
    string workdir;
  };

  enum emExecState {
    ST_RECV_FILE,
    ST_RECV_CONFIG,
    ST_RECV_DIR,
    ST_RECV_CMD,
  };

 public:
  CExecLoader(CWaiter *p_owner);
  virtual ~CExecLoader();

  int run(string &pkg);

  int load();

  static void child_cb(EV_P_ ev_child *w, int revents);

  static void timeout_cb(EV_P_ ev_timer *w, int revents);

 private:
  bool checkTmpDir(string dir_path);

  void runChild(string file_path);

 private:
  string conf_;  // strategy configuration in json format
  string args_;
  string workdir_;  // strategy work directory (i.e., running directory)

  int exec_state_;
  unique_ptr<CFileReciever> p_conf_reciever_;
  unique_ptr<CDirReciever> p_dir_reciever_;
};

#endif /* SRC_TA_CEXECLOADER_H_ */
