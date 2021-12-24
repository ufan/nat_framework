/*
 * CStrategyShared.cpp
 *
 *  Created on: May 22, 2018
 *      Author: hongxu
 */

#include "StrategyShared.h"

#include <string>

#include "json.hpp"
using namespace std;
using json = nlohmann::json;

// record pointer to the this running strategy
static tStrategyNode *s_p_node = nullptr;
static string s_name;    // name of this strategy process
static string s_config;  // configuration of this strategy process

// This flag controls which method is used to manage the strategy
static bool s_use_shm_controller = false;

// Method 1: use a shm table for central stg management of all strategy
// process running in the same host. This table is ignore if not used.
static SHARED_STG_TABLE s_table;
// Method 2: use a private record to manage this strategy process by itself.
// It is ignored if shm table is used.
static tStrategyNode s_stg_node;

/**
 * @brief Init the record of a new strategy process
 * @detail If using method 1 (shm table), this record is managed centrally by
 * StgManager. */
bool initStrategyShared(string name) {
  static bool s_is_init_ = false;
  if (!s_is_init_) {
    s_name = name;
    s_p_node = &s_stg_node;      // record is locally managed, by default
    if (s_use_shm_controller) {  // record managed centrally in a shm table
      if (!SHARED_STG_INIT_TABLE(s_table)) return false;
      if (!registerStg(name)) return false;
      SHARED_STG_TNODE *p = s_table.get(SHARED_STG_STR_TO_KEY(name));
      s_p_node = &(p->val);
    }
    s_p_node->is_exit = 0;
    s_p_node->do_trade = 1;
    s_p_node->userdata = 0;

    s_is_init_ = true;
  }
  return true;
}

string getStrategyName() { return s_name; }

tStrategyNode *getSharedData() { return s_p_node; }

void setStrategyConfig(string content) {
  json j_conf = json::parse(content);
  if (j_conf.find("use_shm_controller") != j_conf.end() &&
      j_conf["use_shm_controller"].get<bool>() == true) {
    s_use_shm_controller = true;
  }
  s_config = content;
}

string getStrategyConfig() { return s_config; }

bool registerStg(string name) {
  tStrategyNode node;
  strncpy(node.name, name.c_str(), SHARED_STG_NAME_MAX_SIZE);
  node.name[SHARED_STG_NAME_MAX_SIZE - 1] = '\0';

  node.start_time = time(NULL);
  node.is_exit = 0;
  node.do_trade = 1;
  node.userdata = 0;

  return s_table.insert(SHARED_STG_STR_TO_KEY(name), node);
}

bool delStg(string name) {
  if (s_use_shm_controller) {
    return s_table.del(SHARED_STG_STR_TO_KEY(name));
  }
  return true;
}
