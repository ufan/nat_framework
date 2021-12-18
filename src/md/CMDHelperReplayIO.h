/*
 * CMDHelperReplayIO.h
 *
 *  Created on: 2018年5月14日
 *      Author: hongxu
 *
 * Comment by Yong:
 * CMDHelperReplayIO is used to replay the quote flow directly from md io pages.
 * Clock time is aligned to the history timeflow.
 * Data source specification useing JSON:
 * - md io path : 'path'
 * - time range : 'start' to 'end' (like 20200504-10:30:00)
 */

#ifndef SRC_MD_CMDHELPERREPLAYIO_H_
#define SRC_MD_CMDHELPERREPLAYIO_H_

#include "CRawIOReader.h"
#include "IMDHelper.h"

class CMDHelperReplayIO : public IMDHelper {
 public:
  using IMDHelper::IMDHelper;
  virtual ~CMDHelperReplayIO() { release(); }

  virtual bool init(const json& j_conf);
  bool _init(string path, string start, string end);

  virtual const UnitedMarketData* read(long& md_nano);

  virtual void release();

  virtual vector<string> getEngineSubscribedInstrument();

  virtual bool doSubscribe(const vector<string>& instr);

  virtual bool doUnsubscribe(const vector<string>& instr) { return true; }

  virtual void setReadPos(long nano) { reader_.setReadPos(nano); }

  bool scan();

  const UnitedMarketData* Next();

 protected:
  bool has_finish_ = false;
  long end_nano_ = 0;
  bool first_tick_ = true;
  CRawIOReader reader_;
};

#endif /* SRC_MD_CMDHELPERREPLAYIO_H_ */
