/*
 * CMDHelperReplayIO.h
 *
 *  Created on: 2018年5月14日
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDHELPERREPLAYIO_H_
#define SRC_MD_CMDHELPERREPLAYIO_H_

#include "IMDHelper.h"
#include "CRawIOReader.h"

class CMDHelperReplayIO: public IMDHelper
{
public:
	using IMDHelper::IMDHelper;
	virtual ~CMDHelperReplayIO() {release();}

	virtual bool init(const json& j_conf);
	bool _init(string path, string start, string end);

	virtual const UnitedMarketData* read(long &md_nano);

	virtual void release();

	virtual vector<string> getEngineSubscribedInstrument();

    virtual bool doSubscribe(const vector<string> &instr);

    virtual bool doUnsubscribe(const vector<string> &instr) {return true;}

    virtual void setReadPos(long nano) {reader_.setReadPos(nano);}

	bool scan();

	const UnitedMarketData* Next();

protected:
	bool			has_finish_ = false;
	long			end_nano_	= 0;
	bool			first_tick_ = true;
	CRawIOReader	reader_;
};

#endif /* SRC_MD_CMDHELPERREPLAYIO_H_ */
