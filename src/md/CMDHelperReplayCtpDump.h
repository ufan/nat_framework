/*
 * CMDHelperReplayCtpDump.h
 *
 *  Created on: 2018年8月8日
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDHELPERREPLAYCTPDUMP_H_
#define SRC_MD_CMDHELPERREPLAYCTPDUMP_H_

#include "IMDHelper.h"
#include "CWareHouseReader.h"

class CMDHelperReplayCtpDump: public IMDHelper, public CWareHouseReader
{
public:
	using IMDHelper::IMDHelper;
	virtual ~CMDHelperReplayCtpDump() {release();}

	virtual bool init(const json& j_conf);

	virtual const UnitedMarketData* read(long &md_nano);

	virtual void release();

	virtual vector<string> getEngineSubscribedInstrument();

    virtual bool doSubscribe(const vector<string> &instr);

    virtual bool doUnsubscribe(const vector<string> &instr) {return true;}

    virtual void setReadPos(long nano) {CWareHouseReader::setReadPos(nano);}

	virtual void onSwitchDay(string instrInfo);

protected:
	bool	has_finish_ = false;
	long 	last_nano_ 	= 0;
};

#endif /* SRC_MD_CMDHELPERREPLAYCTPDUMP_H_ */
