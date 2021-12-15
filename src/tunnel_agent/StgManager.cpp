/*
 * StgManager.cpp
 *
 *  Created on: 2018年4月4日
 *      Author: hongxu
 *
 *  Register, delete and list running strategies in the shm table of the system
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "strategy_shared_comm.h"
#include "utils.h"
using namespace std;

static SHARED_STG_TABLE g_table;

bool registerStg(string name) {
  tStrategyNode node;
  strncpy(node.name, name.c_str(), SHARED_STG_NAME_MAX_SIZE);
  node.name[SHARED_STG_NAME_MAX_SIZE - 1] = '\0';

  node.start_time = time(NULL);
  node.is_exit = 0;
  node.do_trade = 1;
  node.userdata = 0;

  return g_table.insert(SHARED_STG_STR_TO_KEY(name), node);
}

void listStg() {
  char buf[100];
  int totcnt = 0;
  SHARED_STG_TNODE *p_node = NULL;

  g_table.first();
  while (NULL != (p_node = g_table.next())) {
    // key=0 indicates empty entry
    if (p_node->key == 0) continue;

    // start time of the strategy
    string echo_content;
    time_t start_time = p_node->val.start_time;

    echo_content += p_node->val.name;
    struct tm result;
    localtime_r(&start_time, &result);
    snprintf(buf, sizeof(buf), "\t\t\t%d%02d%02d %02d:%02d:%02d",
             result.tm_year + 1900, result.tm_mon + 1, result.tm_mday,
             result.tm_hour, result.tm_min, result.tm_sec);
    echo_content += buf;

    // current running state of the strategy
    if (p_node->val.do_trade)
      echo_content += "\t" GREEN "[TRADING]\t";
    else
      echo_content += "\t" YELLOW "[STOP_TRADE]\t";
    totcnt++;

    snprintf(buf, sizeof(buf), "[%llu]" NOCOLOR,
             (unsigned long long)p_node->val.userdata);
    echo_content += buf;
    cout << echo_content << endl;
  }
}

bool delStg(string name) { return g_table.del(SHARED_STG_STR_TO_KEY(name)); }

void usage(const char *cmd) {
  printf("Usage: %s reg strategy_name|list\n", cmd);
  exit(-1);
}

int main(int argc, char *argv[]) {
  if (!SHARED_STG_INIT_TABLE(g_table)) {
    cerr << "init stg table err." << endl;
    return -1;
  }

  if (argc == 2) {
    if (strcasecmp(argv[1], "list") == 0) {
      listStg();
      return 0;
    }
  } else if (argc == 3) {
    if (strcasecmp(argv[1], "reg") == 0) {
      if (!registerStg(argv[2])) {
        cerr << "register strategy " << argv[2] << " failed!" << endl;
        return -1;
      } else {
        cout << "register strategy " << argv[2] << " succ!" << endl;
        return 0;
      }
    } else if (strcasecmp(argv[1], "del") == 0) {
      if (!delStg(argv[2])) {
        cerr << "delete strategy " << argv[2] << " failed!" << endl;
        return -1;
      } else {
        cout << "delete strategy " << argv[2] << " succ!" << endl;
        return 0;
      }
    }
  }

  usage(argv[0]);
  return -1;
}
