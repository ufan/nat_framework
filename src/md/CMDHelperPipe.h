/*
 * CMDHelperPipe.h
 *
 *  Created on: Aug 29, 2018
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDHELPERPIPE_H_
#define SRC_MD_CMDHELPERPIPE_H_

#include "IMDHelper.h"
#include "CPipExecutor.h"


class CMDHelperPipe: public IMDHelper
{
public:
	using IMDHelper::IMDHelper;
	virtual ~CMDHelperPipe() {release();}

	virtual bool init(const json& j_conf);

	virtual const UnitedMarketData* read(long &md_nano);

	virtual void release();

	virtual vector<string> getEngineSubscribedInstrument();

    virtual bool doSubscribe(const vector<string> &instr);

    virtual bool doUnsubscribe(const vector<string> &instr);

    virtual void setReadPos(long nano);

    void writeJson(const json& j);

    json readJson();

protected:
    CPipExecutor 		*p_executor_ = nullptr;
    UnitedMarketData	res_;
    bool				has_read_	 = false;
    bool				has_finish_  = false;
};

#endif /* SRC_MD_CMDHELPERPIPE_H_ */
