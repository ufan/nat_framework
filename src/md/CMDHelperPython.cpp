/*
 * CMDHelperPython.cpp
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 */

#include "CMDHelperPython.h"
#include "Logger.h"
#include "ATSysUtils.h"
#include "CTradeBaseInfo.h"
#include "CTimer.h"
#include "StrategyShared.h"

extern int g_replay_md_fake_wait_count;

const UnitedMarketData* CMDHelperPython::read(long &md_nano)
{
	if(has_finish_) return nullptr;
	if(has_read_ && g_replay_md_fake_wait_count-- > 0)
	{
		return nullptr;
	}

	long nano = 0;
	const UnitedMarketData* p = exch_.readMd(nano);
	if(p)
	{
		if(filter(p))
		{
			return nullptr;
		}
		if(!has_read_)
		{
			CTimer::instance().setTime(p->exch_time);
			has_read_ = true;
		}
		else if(CTimer::instance().getNano() < p->exch_time)
		{
			CTimer::instance().setTime(p->exch_time);
		}
		g_replay_md_fake_wait_count = 50;
		md_nano = nano;
	}
	else
	{
		has_finish_ = true;
		getSharedData()->is_exit = 1;
		ENGLOG("replay data finished.");
	}
	return p;
}

