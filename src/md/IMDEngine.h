/*
 * IMDEngine.h
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#ifndef SRC_MD_IMDENGINE_H_
#define SRC_MD_IMDENGINE_H_

#include <map>
#include <string>
#include <vector>

#include "CRawIOWriter.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

class IMDEngine {
 public:
  enum emDayNightMode {
    MODE_DAY,
    MODE_NIGHT,
  };

  IMDEngine();
  virtual ~IMDEngine();

  string name() { return name_; }

  vector<string> querySubscribedInstrument();  // invoked by runEngine()

  bool initEngine(const json &j_conf);  // by executable

  void runEngine();  // by executable

  void stop() { do_running_ = false; }  // by executable

  void engine_subscribe(const vector<string> &instr);  // invoked by runEngine()

  void engine_unsubscribe(const vector<string> &instr);  // by runEngine()

  void writeStartSignal();  // by runEngine()

  bool getBaseInfo(string td_engine_name, int timeout);  // by initEngine()

 protected:
  // engine initialization work
  virtual bool init() = 0;  // invoked in initEngine

  // work for starting the engine like: connect to the front, user login ...
  virtual bool start() = 0;  // invoked in runEngine

  // if engine implementation is thread-based, join work may be needed
  virtual void join() = 0;  // designed to be used together with release(), but
                            // some implementation do not use it at all (only
                            // invoked in EESOnload)

  // work for releasing resources occupied by the engine when it is stopped
  virtual void release() = 0;  // invoked in runEngine

  // invoked in engine_subscribe
  virtual void subscribe(const vector<string> &instr) = 0;

  // invoked by engine_unsubscribe
  virtual void unsubscribe(const vector<string> &instr) = 0;

 protected:
  // flag controlling the life of engine, only changed by stop()
  volatile bool do_running_ = true;
  uint8_t day_night_mode_ = MODE_DAY;  // day: 6:00->18:00

  // collection of number of users subscribed for each instrument
  // If new user subscribe to an existing instrument, the corresponding counter
  // increments; if this is a new instrument subscribed by the user, this
  // subscription requirement will be sent to MD front. If an user unsubscribe
  // the intrument, its corresponding counter decrement. If no user subscribe to
  // one instrument anymore, the request to unsubscribed it will be sent to the
  // MD front.
  map<string, int> subs_instr_;

  string name_;  // each running engine instance should have its own unique name
  int self_id_ = 0;  // id of this engine, it's hash id from name

  // MD engine memory mapped file, recording anything related to the engine,
  // including all received tick quotes into memory
  CSafeRawIOWriter md_writer_;

  json config_;  // json configuration
};

#endif /* SRC_MD_IMDENGINE_H_ */
