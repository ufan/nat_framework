/*
 * CMDHelperReplayCtpDump.cpp
 *
 *  Created on: 2018年8月8日
 *      Author: hongxu
 */

#include "CMDHelperReplayCtpDump.h"
#include "CTradeBaseInfo.h"
#include "StrategyShared.h"
#include "MurmurHash2.h"
#include "compiler.h"
#include "Logger.h"

extern int g_replay_md_fake_wait_count;

bool CMDHelperReplayCtpDump::init(const json& j_conf)
{
	doSetTimer(true);
	loadFiles(j_conf["file_pattern"], j_conf["start"], j_conf["end"]);
	return not files_.empty();
}

const UnitedMarketData* CMDHelperReplayCtpDump::read(long & md_nano)
{
	if(has_finish_) return nullptr;
	if(g_replay_md_fake_wait_count-- > 0) return nullptr;
	else g_replay_md_fake_wait_count = 50;

	while(not has_finish_)
	{
		auto p = readTick();
		if(unlikely(not p))
		{
			has_finish_ = true;
			getSharedData()->is_exit = 1;
			ENGLOG("replay data finished.");
			return nullptr;
		}

		if(filter(p)) continue;
		else
		{
			md_nano = local_time_;
			return p;
		}
	}
	return nullptr;
}

void CMDHelperReplayCtpDump::release()
{
	CWareHouseReader::clear();
	IMDHelper::release();
}

vector<string> CMDHelperReplayCtpDump::getEngineSubscribedInstrument()
{
	vector<string> res;
	for(auto &kv : CTradeBaseInfo::instr_info_)
	{
		res.push_back(kv.second.instr);
	}
	return res;
}

bool CMDHelperReplayCtpDump::doSubscribe(const vector<string>& instr)
{
	for(const auto& i: instr)
	{
		uint32_t hash = INSTR_STR_TO_HASH(i);
		if(CTradeBaseInfo::instr_info_.count(hash) == 0)
		{
			return false;
		}
	}
	return true;
}

void CMDHelperReplayCtpDump::onSwitchDay(string instrInfo)
{
	if(CTradeBaseInfo::switch_day_cb_)
	{
		CTradeBaseInfo::switch_day_cb_(CTradeBaseInfo::trading_day_);
	}
}
