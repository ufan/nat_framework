/*
 * CTDHelperComm.h
 *
 *  Created on: 2018年5月8日
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDHELPERCOMM_H_
#define SRC_TD_CTDHELPERCOMM_H_

#include "CRawIOReader.h"
#include "CRawIOWriter.h"
#include "IOCommon.h"
#include "ITDHelper.h"

class CTDHelperComm : public ITDHelper {
 public:
  using ITDHelper::ITDHelper;
  virtual ~CTDHelperComm() { release(); }

  /**
   * @brief Create and init the communication IOs (system, td engine, self) and
   * fetch base info and order track from td engine.
   */
  bool init(const json& j_conf);
  bool _init(string engine, int timeout = 3);

  void doSendOrder(int track_id);

  void doDelOrder(int track_id);

  const tRtnMsg* doGetRtn();

  void release() { notifyTDEngineRemove(); }

  virtual bool qryTradeBaseInfo();

  virtual bool qryOrderTrack();

  /**
   * @brief Create the io page path
   */
  string createTdSendPath(string name);

  /**
   * @brief Ask td engine add this helper as client using system io
   * @details After receiving the request, td engine will add a reader
   *          to this helper's io page to its reading pool.
   */
  bool notifyTDEngineAdd();

  /**
   * @brief Ask td engine to remove this helper from its reading pool
   */
  void notifyTDEngineRemove();

 protected:
  int self_id_ = 0;  // hash id from name

  string tdsend_path_;      // this helper's io directory ('strategy' dir)
  CRawIOWriter td_writer_;  // writer for this helper's io
  CRawIOReader td_reader_;  // reader for td engine's io

  int td_engine_id_ = 0;  // td engine id
  int timeout_;
};

#endif /* SRC_TD_CTDHELPERCOMM_H_ */
