/*
 * PyWrapper.h
 *
 *  Created on: May 30, 2018
 *      Author: hongxu
 */

#ifndef SRC_PYEXT_PYWRAPPER_H_
#define SRC_PYEXT_PYWRAPPER_H_

#include "IMDHelper.h"
#include "ITDHelper.h"
#include "CMDHelperComm.h"
#include "CMDHelperReplayIO.h"
#include "CTDHelperComm.h"
#include "CTDHelperFake.h"
#include "PyUtils.h"
#include "CStrategyProcess.h"
#include "CStrategy.h"
#include "FastLogger.h"
#include "CTradeBaseInfo.h"
#include "RiskStg.h"
#include "CWareHouseReader.h"
#include "CRawIOReader.h"

inline uint32_t getInstrHash(string instr) {return INSTR_STR_TO_HASH(instr);}
inline int getStgHash(string stg) {return HASH_STR(stg.c_str());}

class CMDHelperCommWrapper : public CMDHelperComm
{
public:
	using CMDHelperComm::CMDHelperComm;
	bool _init(string engine, long timeout=5, string from=string()) {return CMDHelperComm::_init(engine, timeout, from);}
    bool pySubscribe(bp::list instr) {return subscribe(list_to_vector<string>(instr));}
    bool pyUnsubscribe(bp::list instr) {return unsubscribe(list_to_vector<string>(instr));}
    bool pyForceUnsubscribe(bp::list instr) {return forceUnsubscribe(list_to_vector<string>(instr));}
    bp::list pyGetEngineSubscribedInstrument() {return vector_to_list(getEngineSubscribedInstrument());}
};

class CMDHelperReplayIOWrapper : public CMDHelperReplayIO
{
public:
	using CMDHelperReplayIO::CMDHelperReplayIO;
    bool pySubscribe(bp::list instr) {return subscribe(list_to_vector<string>(instr));}
    bool pyUnsubscribe(bp::list instr) {return unsubscribe(list_to_vector<string>(instr));}
    bp::list pyGetEngineSubscribedInstrument() {return vector_to_list(getEngineSubscribedInstrument());}
};

class AccBaseWrapper : public AccBase
{
public:
	bool init(string name, string j) {return init(name.c_str(), json::parse(j));}
	bp::list pyGetAllPrd() {return vector_to_list(getAllPrd());}
	bp::list pyGetAllInstr() {return vector_to_list(getAllInstr());}
};

class RiskStgWrapper : public RiskStg
{
public:
	bp::list pyGetAllPrd() {return vector_to_list(getAllPrd());}
	bp::list pyGetAllInstr() {return vector_to_list(getAllInstr());}
};

class CPyStrategy : public CStrategy, public bp::wrapper<CPyStrategy>
{
public:
	virtual void on_tick(const UnitedMarketData* pmd) {if (bp::override f = this->get_override("on_tick")) f(pmd);}
	virtual void on_time(long nano) {if (bp::override f = this->get_override("on_time")) f(nano);}
	virtual void on_rtn(const tRtnMsg* prtn) {if (bp::override f = this->get_override("on_rtn")) f(prtn);}
	virtual void on_bar(const Bar* pbar) {if (bp::override f = this->get_override("on_bar")) f(pbar);}
	virtual void on_switch_day(string day) {if (bp::override f = this->get_override("on_switch_day")) f(day);}
	virtual void on_msg(const char *p, uint32_t len) {if (bp::override f = this->get_override("on_msg")) f(bp::str(p, len));}
	const RiskStgWrapper* getAccObj() {return (const RiskStgWrapper*)getAccountObj();}
};

class CPyStrategyBase : public CStrategyBase, public bp::wrapper<CPyStrategyBase>
{
public:
	virtual void on_tick(const UnitedMarketData* pmd) {if (bp::override f = this->get_override("on_tick")) f(pmd);}
	virtual void on_time(long nano) {if (bp::override f = this->get_override("on_time")) f(nano);}
	virtual void on_rtn(const tRtnMsg* prtn) {if (bp::override f = this->get_override("on_rtn")) f(prtn);}
	virtual void on_bar(const Bar* pbar) {if (bp::override f = this->get_override("on_bar")) f(pbar);}
	virtual void on_switch_day(string day) {if (bp::override f = this->get_override("on_switch_day")) f(day);}
	const RiskStgWrapper* getAccObj() {return (const RiskStgWrapper*)getAccountObj();}
};

bp::list getInstrumentInfo();

bp::dict tick2Dict(const UnitedMarketData* pmd);

class CPyWareHouseReader: public CWareHouseReader
{
public:
	bp::dict readOne()
	{
		auto p = readTick();
		if(p) return tick2Dict(p);
		else return bp::dict();
	}
	bp::list read()
	{
		bp::list res;
		auto p = readTick();
		while(p)
		{
			res.append(tick2Dict(p));
			p = readTick();
		}
		return res;
	}
	void pyLoadFiles(bp::list file_patterns, string start_day, string end_day)
	{
		return loadFiles(list_to_vector<string>(file_patterns), start_day, end_day);
	}
};

inline void setSystemTimeNano(long nano)
{
	CTimer::instance().setTime(nano);
}

class CPyRtnReader
{
public:
	CPyRtnReader() {}
	virtual ~CPyRtnReader() {}

	bool init(string prefix, long from_nano) {return reader_.init(prefix, from_nano);}

	bp::tuple getRtn()
	{
		uint32_t len;
		tIOrderRtn *p = (tIOrderRtn*)(reader_.read(len));
		if(p && p->cmd == IO_ORDER_RTN)
		{
			return bp::make_tuple(p->to, p->rtn_msg, getIOFrameHead(p)->nano, p->to);
		}
		return bp::make_tuple(0, *(tRtnMsg*)nullptr, 0, 0);
	}

protected:
	CRawIOReader	reader_;
};

#endif /* SRC_PYEXT_PYWRAPPER_H_ */

