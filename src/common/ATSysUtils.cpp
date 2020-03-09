/*
 * ATSysUtils.cpp
 *
 *  Created on: May 29, 2018
 *      Author: hongxu
 */

#include <memory>
#include "string.h"
#include "ATSysUtils.h"
#include "CSystemIO.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"


string sysRequest(int cmd, int rsp_cmd, int to, int source, int timeout, const void *extra_data, uint32_t extra_len)
{
	static int s_request_id = 0;
	unique_ptr<CRawIOReader> sys_reader(CSystemIO::instance().createReader());

	string buf;
	if(extra_data != nullptr && extra_len > 0)
	{
		buf.resize(sizeof(tSysIOHead) + extra_len);
	}
	else buf.resize(sizeof(tSysIOHead));

	tSysIOHead* p_head = (tSysIOHead*)buf.data();
	p_head->cmd = cmd;
	p_head->to = to;
	p_head->source = source;
	p_head->back_word = ++s_request_id;

	if(extra_data != nullptr && extra_len > 0)
	{
		memcpy(p_head->data, extra_data, extra_len);
	}
	CSystemIO::instance().getWriter().write(p_head, buf.size());
	if(rsp_cmd == IO_UNKNOWN) return nullptr;

	uint32_t end = time(NULL) + timeout;
	while(time(NULL) < end)
	{
		uint32_t len = 0;
		const tSysIOHead *p = (const tSysIOHead*)sys_reader->read(len);
		if(p)
		{
			if(p->cmd == rsp_cmd && p->source == to
					&& p->to == source && p->back_word == s_request_id)
			{
				return string((const char*)p, len);
			}
		}
		else
		{
			usleep(200000);		// 200ms
		}
	}
	return string();
}

json toJsonStr(const UnitedMarketData *p)
{
	json res;
	res["instr_hash"] = p->instr_hash;
	res["last_px"] = p->last_px;
	res["cum_vol"] = p->cum_vol;
	res["cum_turnover"] = p->cum_turnover;
	res["avg_px"] = p->avg_px;
	res["ask_px"] = p->ask_px;
	res["bid_px"] = p->bid_px;
	res["ask_vol"] = p->ask_vol;
	res["bid_vol"] = p->bid_vol;
	res["ask_px2"] = p->ask_px2;
	res["bid_px2"] = p->bid_px2;
	res["ask_vol2"] = p->ask_vol2;
	res["bid_vol2"] = p->bid_vol2;
	res["ask_px3"] = p->ask_px3;
	res["bid_px3"] = p->bid_px3;
	res["ask_vol3"] = p->ask_vol3;
	res["bid_vol3"] = p->bid_vol3;
	res["ask_px4"] = p->ask_px4;
	res["bid_px4"] = p->bid_px4;
	res["ask_vol4"] = p->ask_vol4;
	res["bid_vol4"] = p->bid_vol4;
	res["ask_px5"] = p->ask_px5;
	res["bid_px5"] = p->bid_px5;
	res["ask_vol5"] = p->ask_vol5;
	res["bid_vol5"] = p->bid_vol5;
	res["open_interest"] = p->open_interest;
	res["exch_time"] = p->exch_time;
	res["instr_str"] = p->instr_str;
	return res;
}

void parseJsonStr(const json &j, UnitedMarketData *save)
{
	save->last_px = j["last_px"];
	save->cum_vol = j["cum_vol"];
	save->cum_turnover = j["cum_turnover"];
	save->avg_px = j["avg_px"];
	save->ask_px = j["ask_px"];
	save->bid_px = j["bid_px"];
	save->ask_vol = j["ask_vol"];
	save->bid_vol = j["bid_vol"];
	save->ask_px2 = j["ask_px2"];
	save->bid_px2 = j["bid_px2"];
	save->ask_vol2 = j["ask_vol2"];
	save->bid_vol2 = j["bid_vol2"];
	save->ask_px3 = j["ask_px3"];
	save->bid_px3 = j["bid_px3"];
	save->ask_vol3 = j["ask_vol3"];
	save->bid_vol3 = j["bid_vol3"];
	save->ask_px4 = j["ask_px4"];
	save->bid_px4 = j["bid_px4"];
	save->ask_vol4 = j["ask_vol4"];
	save->bid_vol4 = j["bid_vol4"];
	save->ask_px5 = j["ask_px5"];
	save->bid_px5 = j["bid_px5"];
	save->ask_vol5 = j["ask_vol5"];
	save->bid_vol5 = j["bid_vol5"];
	save->open_interest = j["open_interest"];
	save->exch_time = j["exch_time"];
	save->setInstrStr(j["instr_str"]);
	save->instr_hash = INSTR_NAME_TO_HASH(save->instr_str);
}

json toJson(const tInstrumentInfo *p)
{
	json res;
	res["instr_hash"] = p->instr_hash;
	res["instr"] = p->instr;
	res["exch"] = p->exch;
	res["product"] = p->product;
	res["product_hash"] = p->product_hash;
	res["vol_multiple"] = p->vol_multiple;
	res["tick_price"] = p->tick_price;
	res["expire_date"] = p->expire_date;
	res["is_trading"] = p->is_trading;
	return res;
}

void parseJsonStr(const json &j, tInstrumentInfo *save)
{
	save->setInstrStr(j["instr"]);
	save->instr_hash = INSTR_NAME_TO_HASH(save->instr);
	save->exch = j["exch"];
	save->setProduct(j["product"]);
	save->product_hash = INSTR_NAME_TO_HASH(save->product);
	save->vol_multiple = j["vol_multiple"];
	save->tick_price = j["tick_price"];
	save->setExpireDate(j["expire_date"]);
	save->is_trading = j["is_trading"];
}

json baseInfo2Json()
{
	json res;
	res["trading_day"] = CTradeBaseInfo::trading_day_;
	json &info = res["instr_info"];
	for(const auto &kv : CTradeBaseInfo::instr_info_)
	{
		info.push_back(toJson(&(kv.second)));
	}
	return res;
}

void json2BaseInfo(const json &j)
{
	string day = j["trading_day"];
	if(CTradeBaseInfo::trading_day_ != day)
	{
		CTradeBaseInfo::instr_info_.clear();
		const json &info = j["instr_info"];
		for(auto &i : info)
		{
			tInstrumentInfo instr_info;
			parseJsonStr(i, &instr_info);
			CTradeBaseInfo::instr_info_[instr_info.instr_hash] = instr_info;
		}
		CTradeBaseInfo::trading_day_ = day;
		CTradeBaseInfo::is_init_ = true;

		if(CTradeBaseInfo::switch_day_cb_)
		{
			CTradeBaseInfo::switch_day_cb_(CTradeBaseInfo::trading_day_);
		}
	}
}

void parseJsonStr(const json &j, tRtnMsg *save)
{
	save->msg_type = j["msg_type"];
	save->local_id = j["local_id"];
	save->setInstrStr(j["instr_str"]);
	save->instr_hash = INSTR_NAME_TO_HASH(save->instr);
	save->price = j["price"];
	save->vol = j["vol"];
	save->dir = j["dir"];
	save->off = j["off"];
	save->order_ref = j["order_ref"];
	save->front_id = j["front_id"];
	save->session_id = j["session_id"];
	save->errid = j["errid"];
	save->setMsg(j["msg"]);
}
