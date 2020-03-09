/*
 * CTDHelperPython.cpp
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 */

#include "CTDHelperPython.h"
#include "CTradeBaseInfo.h"

extern int g_replay_md_fake_wait_count;

bool CTDHelperPython::init(const json& j_conf)
{
	if(not exch_.init(j_conf))
	{
		ALERT("init td_exch failed.");
		return false;
	}
	if(!CTradeBaseInfo::is_init_ && !qryTradeBaseInfo())
	{
		ALERT("get td base information failed.");
		return false;
	}
	if(!qryOrderTrack())
	{
		ALERT("query order track failed,");
		return false;
	}
	return true;
}

const tRtnMsg* CTDHelperPython::doGetRtn()
{
	if(const tRtnMsg* prtn = exch_.getRtn())
	{
		g_replay_md_fake_wait_count ++;
		return prtn;
	}
	return nullptr;
}
