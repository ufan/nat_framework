/*
 * CMDHelperPython.h
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDHELPERPYTHON_H_
#define SRC_MD_CMDHELPERPYTHON_H_

#include "IMDHelper.h"
#include "PyExtExch.h"

class CMDHelperPython : public IMDHelper
{
public:
	using IMDHelper::IMDHelper;
	virtual ~CMDHelperPython() {release();}

	virtual bool init(const json& j_conf) {return exch_.init(j_conf);}

	virtual const UnitedMarketData* read(long &md_nano);

	virtual void release() {IMDHelper::release(); exch_.release();}

	virtual vector<string> getEngineSubscribedInstrument() {return exch_.getSubsInstr();}

    virtual bool doSubscribe(const vector<string> &instr) {return exch_.subscribe(instr);}

    virtual bool doUnsubscribe(const vector<string> &instr) {return exch_.unsubscribe(instr);}

    virtual void setReadPos(long nano) {exch_.setReadPos(nano);}

protected:
    CPyExtMdExch		exch_;
    bool				has_read_	 = false;
    bool				has_finish_  = false;
};

#endif /* SRC_MD_CMDHELPERPYTHON_H_ */
