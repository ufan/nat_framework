/*
 * IMDHelper.cpp
 *
 *  Created on: 2018年5月17日
 *      Author: hongxu
 */

#include "IMDHelper.h"
#include "MurmurHash2.h"
#include "CTradeBaseInfo.h"


bool IMDHelper::subscribe(const vector<string> &instr)
{
	auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
	vector<string> real_subs;
	for(const auto& i : instr_set)
	{
		uint32_t hash = INSTR_NAME_TO_HASH(i.c_str());
		auto itr = subs_instr_hash_map_.find(hash);
		if(itr == subs_instr_hash_map_.end())
		{
			real_subs.push_back(i);
			subs_instr_hash_map_[hash] = i;
		}
	}
	return doSubscribe(real_subs);
}

bool IMDHelper::unsubscribe(const vector<string> &instr)
{
	auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
	vector<string> real_unsubs;
	for(const auto& i : instr_set)
	{
		uint32_t hash = INSTR_NAME_TO_HASH(i.c_str());
		auto itr = subs_instr_hash_map_.find(hash);
		if(itr != subs_instr_hash_map_.end())
		{
			real_unsubs.push_back(i);
			subs_instr_hash_map_.erase(hash);
		}
	}
	return doUnsubscribe(real_unsubs);
}

bool IMDHelper::forceUnsubscribe(const vector<string> &instr)
{
	auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
	vector<string> real_unsubs;
	for(const auto& i : instr_set)
	{
		real_unsubs.push_back(i);
		subs_instr_hash_map_.erase(INSTR_NAME_TO_HASH(i.c_str()));
	}
	return doUnsubscribe(real_unsubs);
}

vector<string> IMDHelper::getSubscribedInstrument()
{
	vector<string> res;
	for(auto& kv : subs_instr_hash_map_)
	{
		res.push_back(kv.second);
	}
	return res;
}

void IMDHelper::clear()
{
	auto subs = getSubscribedInstrument();
	doUnsubscribe(subs);
	subs_instr_hash_map_.clear();
}
