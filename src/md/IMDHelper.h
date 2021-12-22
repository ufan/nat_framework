/*
 * IMDHelper.h
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#ifndef SRC_MD_IMDHELPER_H_
#define SRC_MD_IMDHELPER_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ATStructure.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

class IMDHelper {
 public:
  IMDHelper(string name) : name_(name) {}
  virtual ~IMDHelper() {}

  virtual bool init(const json &j_conf) = 0;

  virtual const UnitedMarketData *read(long &md_nano) = 0;

  virtual void release() { clear(); }

  virtual vector<string> getEngineSubscribedInstrument() = 0;

  virtual bool doSubscribe(const vector<string> &instr) = 0;

  virtual bool doUnsubscribe(const vector<string> &instr) = 0;

  virtual void setReadPos(long nano) = 0;

  bool subscribe(const vector<string> &instr);

  bool unsubscribe(const vector<string> &instr);

  bool forceUnsubscribe(const vector<string> &instr);

  bool reSubscribe() { return doSubscribe(getSubscribedInstrument()); }

  vector<string> getSubscribedInstrument();

  void clear();
  void clearMap() { subs_instr_hash_map_.clear(); }

  /**
   * @brief Whether the tick belongs to the subscribed instruments
   * Return true, if not
   */
  bool filter(const UnitedMarketData *pmd) {
    return subs_instr_hash_map_.count(pmd->instr_hash) == 0;
  }

  const char *getInstrNameByHash(uint32_t hash) {
    auto itr = subs_instr_hash_map_.find(hash);
    if (itr != subs_instr_hash_map_.end()) {
      return itr->second.c_str();
    }
    return nullptr;
  }

  bool initStr(string conf) { return init(json::parse(conf)); }

  const UnitedMarketData *read() {
    long nano;
    return read(nano);
  }

 protected:
  // the collection of subscribed instruments
  // Value is the name of instrument, Key is the hash id of value
  unordered_map<uint32_t, string> subs_instr_hash_map_;

  string name_;  // unique name of the helper in system
};

typedef unique_ptr<IMDHelper> IMDHelperPtr;

#endif /* SRC_MD_IMDHELPER_H_ */
