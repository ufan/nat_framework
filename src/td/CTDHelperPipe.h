/*
 * CTDHelperPipe.h
 *
 *  Created on: Sep 7, 2018
 *      Author: hongxu
 *
 * Comment by Yong:
 * This helper provides an interface to other implementations of TD service.
 * The external TD is used for relay or simulation purpose.
 * TD executable is ran as child process of the running strategy.
 * Communication is handled through three pipes between the parent and child
 * process. External TD implementation needs to read command from the std input,
 * write response to the std output and write the return message for order
 * update to the std error.
 *
 * The message string is based JSON format, with '\n' as the last charactor
 * indicating end of message.
 * Any executable implementing these interfaces can be connected with this
 * helper.
 * An example TD implementation is YTExch.py, which replays the warehouse data.
 *
 */

#ifndef SRC_TD_CTDHELPERPIPE_H_
#define SRC_TD_CTDHELPERPIPE_H_

#include <queue>

#include "CPipExecutor.h"
#include "ITDHelper.h"

class CTDHelperPipe : public ITDHelper {
 public:
  using ITDHelper::ITDHelper;
  virtual ~CTDHelperPipe() { release(); }

  bool init(const json& j_conf);

  void doSendOrder(int track_id);

  void doDelOrder(int track_id);

  const tRtnMsg* doGetRtn();

  void release();

  virtual bool qryTradeBaseInfo();

  virtual bool qryOrderTrack();

  void writeJson(const json& j);

  json readJson();

 protected:
  tRtnMsg hold_result_;
  CPipExecutor* p_executor_ = nullptr;
  int self_id_ = 0;
  queue<tRtnMsg> rtn_msg_queue_;
};

#endif /* SRC_TD_CTDHELPERPIPE_H_ */
