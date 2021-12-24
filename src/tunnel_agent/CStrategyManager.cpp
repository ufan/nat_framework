/*
 * CStrategyManager.cpp
 *
 *  Created on: 2017年10月19日
 *      Author: hongxu
 */

#include "CStrategyManager.h"

#include <signal.h>
#include <time.h>

#include "CWaiter.h"
#include "utils.h"

bool CStrategyManager::s_is_init = false;
SHARED_STG_TABLE CStrategyManager::s_table_;

CStrategyManager::CStrategyManager(CWaiter* p_owner)
    : CCommanderBase(p_owner) {}

CStrategyManager::~CStrategyManager() {}

int CStrategyManager::run(string& pkg) {
  int ret = STATE_FINISH;

  if (pkg.size() < sizeof(tStgCommand)) {
    p_owner_->sayFinish(TYPE_FINISH_FAIL);
    return STATE_ABORT;
  }

  tStgCommand* p_cmd = (tStgCommand*)pkg.data();
  string name(p_cmd->name, pkg.size() - sizeof(tStgCommand));
  switch (p_cmd->type) {
    case STG_STATUS:
      ret = status();
      break;
    case STG_START_TRADE: {
      SHARED_STG_TNODE* p = getStg(name);
      if (p) p->val.do_trade = 1;
      break;
    }
    case STG_STOP_TRADE: {
      SHARED_STG_TNODE* p = getStg(name);
      if (p) p->val.do_trade = 0;
      break;
    }
    case STG_EXIT: {
      SHARED_STG_TNODE* p = getStg(name);
      if (p) p->val.is_exit = 1;
      break;
    }
    case STG_SIGNAL: {
      pid_t pid = findProcessByCmdLine(name);
      if (pid >= 0) {
        // higher 32 bits: signal data
        union sigval mysigval;
        mysigval.sival_int = (int)(p_cmd->data >> 32);

        // lower 32 bits: signal id
        // NOTE: 1. compiler will cast 64bits to the lower 32bits
        //       2. supplement 32-bit data can be sent together with signal
        //       using sigqueue, but the receiver must use sigaction to install
        //       signal handler and set SA_SIGINFO flag.
        //       3. real-time signal sent by sigqueue will guaranteed to be
        //       queued (kill-sent signal may not)
        sigqueue(pid, (int)(p_cmd->data), mysigval);
      }
      break;
    }
    case STG_SET_DATA: {
      SHARED_STG_TNODE* p = getStg(name);
      if (p) p->val.userdata = p_cmd->data;
      break;
    }
    default:;
  }

  if (ret == STATE_FINISH) {
    p_owner_->sayFinish();
  } else if (ret == STATE_ABORT) {
    p_owner_->sayFinish(TYPE_FINISH_FAIL);
  }
  return ret;
}

// Generate a report about current status of all running strategies on the host
// machine It will also check if the strategy process exits abnormally, report
// it, then delete the related record from the table
int CStrategyManager::status() {
  char buf[100];
  string echo_content;
  int totcnt = 0;
  SHARED_STG_TNODE* p_node = NULL;

  s_table_.first();
  while (NULL != (p_node = s_table_.next())) {
    if (p_node->key == 0) continue;

    time_t start_time = p_node->val.start_time;
    if (time(NULL) - start_time < 2)
      continue;  // don't list strategy started in last 2s.

    echo_content += p_node->val.name;
    struct tm result;
    localtime_r(&start_time, &result);
    snprintf(buf, sizeof(buf), "\t\t\t%d%02d%02d %02d:%02d:%02d",
             result.tm_year + 1900, result.tm_mon + 1, result.tm_mday,
             result.tm_hour, result.tm_min, result.tm_sec);
    echo_content += buf;

    if (!checkStgExist(p_node->val.name)) {  // record exist but process not,
                                             // report it and delete the record
      s_table_.del(p_node->key);
      echo_content += "\t" YELLOW "[UNNORMAL_EXIT]\t";
    } else {
      if (p_node->val.do_trade)
        echo_content += "\t" GREEN "[TRADING]\t";
      else
        echo_content += "\t" YELLOW "[STOP_TRADE]\t";
      totcnt++;
    }
    snprintf(buf, sizeof(buf), "[%llu]" NOCOLOR "\n",
             (unsigned long long)p_node->val.userdata);
    echo_content += buf;
  }

  snprintf(buf, sizeof(buf), "total " SOK("[%d]") " strategy running.\n",
           totcnt);
  echo_content += buf;

  sendToClient(echo_content);
  return STATE_FINISH;
}

// Attach to shared memory containing the strategy table to the address space of
// the owner process
bool CStrategyManager::commInit() {
  if (!s_is_init) {
    if (!SHARED_STG_INIT_TABLE(s_table_)) return false;
    s_is_init = true;
  }
  return true;
}

// Insert a new entry in the shm table if strategy of name not exist in the
// table Otherwise, reset the exiting record with fresh value
bool CStrategyManager::registerStg(string name) {
  tStrategyNode node;
  strncpy(node.name, name.c_str(), SHARED_STG_NAME_MAX_SIZE);
  node.name[SHARED_STG_NAME_MAX_SIZE - 1] = '\0';

  node.start_time = time(NULL);
  node.is_exit = 0;
  node.do_trade = 1;
  node.userdata = 0;

  return s_table_.insert(SHARED_STG_STR_TO_KEY(name), node);
}

// Is strategy still running: used in abnormal exit checking
bool CStrategyManager::checkStgExist(string name) {
  pid_t pid = findProcessByCmdLine(name);
  if (pid < 0) return false;
  return true;
}

// Delete the record of the strategy with 'name' from the table
bool CStrategyManager::exitStg(string name) {
  return s_table_.del(SHARED_STG_STR_TO_KEY(name));
}

// Return the pointer to an exiting strategy record with 'name'
// return nullptr if no record exist in the table
SHARED_STG_TNODE* CStrategyManager::getStg(string name) {
  return s_table_.get(SHARED_STG_STR_TO_KEY(name));
}
