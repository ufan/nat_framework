/*
 * CDemo.cpp
 *
 *  Created on: May 25, 2018
 *      Author: hongxu
 */

#include "CDemo.h"
#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "CEncodeConv.h"
#include "CRawIOReader.h"

CDemo::CDemo()
{

}

CDemo::~CDemo()
{

}

void CDemo::on_tick(const UnitedMarketData* pmd)
{
	static int s_cnt = 50;
	string nano = parseNano(pmd->exch_time, "%Y%m%d-%H:%M:%S");
	char *p = (char*)pmd - offsetof(tIOMarketData, market_data);
	string ionano = parseNano(getIOFrameHead(p)->nano, "%Y%m%d-%H:%M:%S");

	printf("%s|exch_time:%s %s|%u price:%lf vol:%d ask:%lf ask_vol:%d bid:%lf bid_vol:%d\n",
			ionano.c_str(), nano.c_str(), pmd->instr_str, pmd->instr_hash, pmd->last_px, pmd->cum_vol, pmd->ask_px, pmd->ask_vol, pmd->bid_px, pmd->bid_vol);

	if(s_cnt-- <= 0) return;
	//int id = sendOrder(pmd->instr_hash, pmd->ask_px - 5000, pmd->ask_vol, AT_CHAR_Buy, AT_CHAR_Open);
	//printf("sendOrder id: %d\n", id);
}

void CDemo::on_time(long nano)
{
	static long last_nano = 0;
	if(nano > last_nano + 5000000000L && false)
	{
		last_nano = nano;
		string snano = parseNano(nano, "%Y%m%d-%H:%M:%S");
		cout << "cur_nano: " << snano << endl;
	}
}

void CDemo::on_rtn(const tRtnMsg* rtn)
{
	string msg = CEncodeConv::gbk2utf8(rtn->msg);
	printf("get return msg %d: %s price:%lf vol:%d msg:%s\n", static_cast<int>(rtn->msg_type), rtn->instr, rtn->price, rtn->vol, msg.c_str());
}

void CDemo::on_switch_day(string day)
{
	printf("on_switch_day:%s\n", day.c_str());
	subscribe("all");
}

int main(int argc, char *argv[])
{
	CDemo demo;
	if(!demo.init(argv[1]))
	{
		cerr << "strategy init err" << endl;
		return -1;
	}

	cout << "trading_day: " << CTradeBaseInfo::trading_day_ << endl;
	cout << "instr_cnt: " << CTradeBaseInfo::instr_info_.size() << endl;
	for(auto &kv : CTradeBaseInfo::instr_info_)
	{
	//	cout << kv.second.instr << " " << kv.second.product << endl;
	}

	demo.run();
	demo.release();

	return 0;
}
