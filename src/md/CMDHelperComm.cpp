/*
 * CMDHelperComm.cpp
 *
 *  Created on: 2018年5月11日
 *      Author: hongxu
 */

#include <memory>
#include "CMDHelperComm.h"
#include "CSystemIO.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "ATSysUtils.h"
#include "CTradeBaseInfo.h"


bool CMDHelperComm::init(const json& j_conf)
{
	string from;
	if(j_conf.find("from") != j_conf.end())
	{
		from = j_conf["from"];
	}
	return _init(j_conf["md_engine_name"], j_conf["timeout"], from);
}

bool CMDHelperComm::_init(string engine, long timeout, string from)
{
	if(!CSystemIO::instance().init())
	{
		ALERT("init system io writer err.");
		return false;
	}

	self_id_ = (int)HASH_STR(name_.c_str());
	md_engine_id_ = (int)HASH_STR(engine.c_str());
	timeout_ = timeout;

	if(!testHeatBeat()) return false;
	if(!qryTradeBaseInfo())
	{
		ALERT("get base information failed.");
		return false;
	}

	string path = string(IO_MDENGINE_BASE_PATH) + engine + "/md";

	long start = -1;
	if(from.size())
	{
		start = parseTime(from.c_str(), "%Y%m%d-%H:%M:%S");
	}
	return reader_.init(path, start);
}

bool CMDHelperComm::testHeatBeat()
{
	if(sysRequest(IO_HEAT_BEAT, IO_HEAT_BEAT_ACK, md_engine_id_, self_id_, timeout_).size()) return true;
	ALERT("connect to md_engine timeout.");
	return false;
}

vector<string> CMDHelperComm::getEngineSubscribedInstrument()
{
	auto p = sysRequest(IO_QUERY_SUBS_INSTR, IO_RSP_QUERY_SUBS_INSTR, md_engine_id_, self_id_, timeout_);
	if(p.size())
	{
		try
		{
			auto j = json::parse(((tSysIOHead*)p.data())->data);
			vector<string> res = j;
			return res;
		}
		catch(...)
		{
			ALERT("json parse err: %s", ((tSysIOHead*)p.data())->data);
			return {};
		}
	}

	ALERT("query md_engine subscribed instrument timeout");
	return {};
}

bool CMDHelperComm::doSubscribe(const vector<string> &instr)
{
	if(instr.size())
	{
		json j(instr);
		string data = j.dump();
		auto p = sysRequest(IO_SUBS_INSTR, IO_ACK, md_engine_id_, self_id_, timeout_, data.c_str(), data.size() + 1);
		if(p.empty())
		{
			ALERT("subscribe instrument timeout");
			return false;
		}
	}
	return true;
}

bool CMDHelperComm::doUnsubscribe(const vector<string> &instr)
{
	if(instr.size())
	{
		json j(instr);
		string data = j.dump();
		auto p = sysRequest(IO_UNSUBS_INSTR, IO_ACK, md_engine_id_, self_id_, timeout_, data.c_str(), data.size() + 1);
		if(p.empty())
		{
			ALERT("unsubscribe instrument timeout");
			return false;
		}
	}
	return true;
}

bool CMDHelperComm::qryTradeBaseInfo(bool &update)
{
	string res = sysRequest(IO_TD_REQ_BASE_INFO, IO_TD_RSP_BASE_INFO, md_engine_id_, self_id_, timeout_);
	if(res.size())
	{
		const tIOTDBaseInfo *p = (const tIOTDBaseInfo*)((const tSysIOHead*)res.data())->data;
		bool ret = CTradeBaseInfo::update(p);
		if(&update != nullptr) update = ret;
		return true;
	}
	return false;
}

void CMDHelperComm::release()
{
	IMDHelper::release();
	reader_.unload();
}
