/*
 * CDemoStrategy.cpp
 *
 *  Created on: 2018年5月10日
 *      Author: hongxu
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include "CDemoStrategy.h"
#include "CTimer.h"
using namespace std;

CDemoStrategy::CDemoStrategy()
{

}

CDemoStrategy::~CDemoStrategy()
{

}

void CDemoStrategy::on_tick(const UnitedMarketData* pmd)
{
	static int s_cnt = 50;
	string nano = parseNano(pmd->exch_time, "%Y%m%d-%H:%M:%S");
	printf("tick: time:%s %s|%u price:%lf vol:%d ask:%lf ask_vol:%d bid:%lf bid_vol:%d\n",
			nano.c_str(), pmd->instr_str, pmd->instr_hash, pmd->last_px, pmd->cum_vol, pmd->ask_px, pmd->ask_vol, pmd->bid_px, pmd->bid_vol);

	if(s_cnt-- <= 0) return;
	int id = sendOrder(pmd->instr_hash, pmd->ask_px - 2, pmd->ask_vol, AT_CHAR_Buy, AT_CHAR_Open);
	printf("sendOrder id: %d\n", id);
}

void CDemoStrategy::on_time(long nano)
{
	static long last_nano = 0;
	if(nano > last_nano + 5000000000L)
	{
		last_nano = nano;
		string snano = parseNano(nano, "%Y%m%d-%H:%M:%S");
		cout << "cur_nano: " << snano << endl;
	}
}

void CDemoStrategy::on_rtn(const tRtnMsg* rtn)
{
	printf("get return msg %d: %s price:%lf vol:%d\n", static_cast<int>(rtn->msg_type), rtn->instr, rtn->price, rtn->vol);
}

int main(int argc, char *argv[])
{
	if(!CStrategyProcess::init(argv[1]))
	{
		cerr << "process init err" << endl;
		return -1;
	}

	CDemoStrategy stg;
	if(stg.activate() < 0)
	{
		cerr << "active strategy err" << endl;
		return -1;
	}
	stg.subscribe("rb1810");

	CStrategyProcess::run();
	CStrategyProcess::release();

	return 0;
}

