/*
 * CMDHelperPipe.cpp
 *
 *  Created on: Aug 29, 2018
 *      Author: hongxu
 */

#include "CMDHelperPipe.h"
#include "Logger.h"
#include "ATSysUtils.h"
#include "CTradeBaseInfo.h"
#include "CTimer.h"
#include "StrategyShared.h"

extern int g_replay_md_fake_wait_count;

bool CMDHelperPipe::init(const json& j_conf)
{
	p_executor_ = CPipExecutorManager::instance().create(j_conf["path"], j_conf["args"]);
	if(not p_executor_)
	{
		ALERT("create pipe process failed.");
		return false;
	}

	json j;
	j["cmd"] = "queryBaseInfo";
	writeJson(j);

	try
	{
		auto res = readJson();
		if(res.empty() || res["ret"].get<string>() != "OK") return false;

		json2BaseInfo(res);
	}
	catch(...)
	{
		ALERT("json parse err");
		return false;
	}
	return true;
}

const UnitedMarketData* CMDHelperPipe::read(long & md_nano)
{
	if(has_finish_) return nullptr;
	if(has_read_ && g_replay_md_fake_wait_count-- > 0)
	{
		return nullptr;
	}

	json j;
	j["cmd"] = "read";
	writeJson(j);
	auto res = readJson();
	if(res.empty() || res["ret"].get<string>() != "OK")
	{
		has_finish_ = true;
		getSharedData()->is_exit = 1;
		ENGLOG("replay data finished.");
		return nullptr;
	}

	if(res["type"].get<string>() == "md")
	{
		const json &data = res["data"];
		parseJsonStr(data, &res_);
		if(filter(&res_))
		{
			return nullptr;
		}
		if(!has_read_)
		{
			CTimer::instance().setTime(res_.exch_time);
			has_read_ = true;
		}
		else if(CTimer::instance().getNano() < res_.exch_time)
		{
			CTimer::instance().setTime(res_.exch_time);
		}
		g_replay_md_fake_wait_count = 50;
		md_nano = CTimer::instance().getNano();
		return &res_;
	}
	else if(res["type"].get<string>() == "updateBaseInfo")
	{
		json2BaseInfo(res["data"]);
	}

	return nullptr;
}

void CMDHelperPipe::release()
{
	IMDHelper::release();
	if(p_executor_)
	{
		CPipExecutorManager::instance().kill(p_executor_->getHash());
		p_executor_ = nullptr;
	}
}

vector<string> CMDHelperPipe::getEngineSubscribedInstrument()
{
	json j;
	j["cmd"] = "getEngineSubscribedInstrument";
	writeJson(j);

	auto res = readJson();
	if(res.empty() || res["ret"].get<string>() != "OK") return {};

	return res["instr"];
}

bool CMDHelperPipe::doSubscribe(const vector<string>& instr)
{
	json j;
	j["cmd"] = "subscribe";
	j["instr"] = instr;
	writeJson(j);

	auto res = readJson();
	if(res.empty() || res["ret"].get<string>() != "OK") return false;
	return true;
}

bool CMDHelperPipe::doUnsubscribe(const vector<string>& instr)
{
	json j;
	j["cmd"] = "unsubscribe";
	j["instr"] = instr;
	writeJson(j);

	auto res = readJson();
	if(res.empty() || res["ret"].get<string>() != "OK") return false;
	return true;
}

void CMDHelperPipe::setReadPos(long nano)
{
	json j;
	j["cmd"] = "setReadPos";
	j["nano"] = nano;

	writeJson(j);
}

void CMDHelperPipe::writeJson(const json& j)
{
	if(p_executor_)
	{
		string s = j.dump();
		write(p_executor_->getWriteFd(), s.c_str(), s.size());
		write(p_executor_->getWriteFd(), "\n", 1);
	}
}

json CMDHelperPipe::readJson()
{
	json res;
	if(p_executor_)
	{
		string data;
		char buf[4096];

		int ret = ::read(p_executor_->getReadFd(), buf, sizeof(buf));
		if(ret > 0) data.append(buf, ret);
		else return res;

		while(data.back() != '\n')
		{
			ret = ::read(p_executor_->getReadFd(), buf, sizeof(buf));
			if(ret > 0) data.append(buf, ret);
			else break;
		}

		if(data.back() == '\n')
		{
			res = json::parse(data);
		}
	}
	return res;
}

