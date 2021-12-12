/*
 * CMDHelperComm.h
 *
 *  Created on: 2018年5月10日
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDHELPERCOMM_H_
#define SRC_MD_CMDHELPERCOMM_H_

#include "CRawIOReader.h"
#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "IMDHelper.h"
#include "IOCommon.h"
#include "SysConf.h"

class CMDHelperComm : public IMDHelper {
 public:
  using IMDHelper::IMDHelper;
  virtual ~CMDHelperComm() { release(); }

  virtual bool init(const json &j_conf);
  bool _init(string engine, long timeout = 5, string from = string());

  virtual const UnitedMarketData *read(long &md_nano) {
    while (true) {
      uint32_t len;
      const tIOMarketData *p = (const tIOMarketData *)reader_.read(len);
      if (p) {
        if (p->cmd == IO_MARKET_DATA) {
          auto pmd = &(p->market_data);
          if (not filter(pmd)) {  // filter out subscribed instruments
            md_nano = getIOFrameHead(p)->nano;
            return pmd;
          }
        } else if (p->cmd == IO_MD_START) {
          if (getIOFrameHead(p)->nano >
              CTimer::instance().getNano() - 5000000000L) {
            bool update;
            if (qryTradeBaseInfo(update)) {  // TBU
              if (!update) {
                reSubscribe();
              }
            }
          }
        } else if (p->cmd == IO_TD_RSP_BASE_INFO) {
          CTradeBaseInfo::update((tIOTDBaseInfo *)((tSysIOHead *)p)->data);
        }
      } else
        break;
    }
    return nullptr;
  }

  virtual void release();

  virtual vector<string> getEngineSubscribedInstrument();

  virtual bool doSubscribe(const vector<string> &instr);

  virtual bool doUnsubscribe(const vector<string> &instr);

  virtual void setReadPos(long nano) { reader_.setReadPos(nano); }

  bool testHeatBeat();

  bool qryTradeBaseInfo(bool &update = *(bool *)nullptr);

 protected:
  int self_id_ = 0;  // hash of name

  CRawIOReader reader_;   // reading md engine io
  int md_engine_id_ = 0;  // id of the connected md engine
  long timeout_ = 5;
};

#endif /* SRC_MD_CMDHELPERCOMM_H_ */
