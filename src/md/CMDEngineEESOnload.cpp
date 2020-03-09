/*
 * CMDEngineEESOnload.cpp
 *
 *  Created on: Aug 1, 2018
 *      Author: hongxu
 */

#include <thread>
#include <functional>
#include "CMDEngineEESOnload.h"
#include "CEESOnload.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "utils.h"

CMDEngineEESOnload CMDEngineEESOnload::instance_;
static CEESOnload md_processor;
static std::thread md_thread;

static void getMcMd(const struct guava_udp_normal *p);

CMDEngineEESOnload::CMDEngineEESOnload()
{

}

CMDEngineEESOnload::~CMDEngineEESOnload()
{

}

bool CMDEngineEESOnload::init()
{
	name_ = config_["/OnloadEESMD/name"_json_pointer];
	return md_processor.init(config_["/OnloadEESMD/interface"_json_pointer],
			config_["/OnloadEESMD/mc_ip"_json_pointer],
			config_["/OnloadEESMD/mc_port"_json_pointer]);
}

bool CMDEngineEESOnload::start()
{
	md_processor.registerMDCallBack(getMcMd);
	md_thread = thread(&CEESOnload::start, &md_processor);
	return true;
}

void CMDEngineEESOnload::join()
{
	md_thread.join();
}

void CMDEngineEESOnload::release()
{
	if(md_thread.joinable())
	{
		md_processor.stop();
		join();
	}
}

void CMDEngineEESOnload::subscribe(const vector<string>& instr)
{

}

void CMDEngineEESOnload::unsubscribe(const vector<string>& instr)
{

}

inline void CMDEngineEESOnload::processMd(const void *_p)
{
	const struct guava_udp_normal *p_data = (const struct guava_udp_normal *)_p;

	tIOMarketData *io = (tIOMarketData*)md_writer_.prefetch(sizeof(tIOMarketData));
	io->cmd = IO_MARKET_DATA;
	UnitedMarketData &p = io->market_data;
	p.instr_hash = INSTR_NAME_TO_HASH(p_data->m_symbol);
	p.last_px = p_data->m_last_px;
	p.cum_vol = p_data->m_last_share;			// notice this
	p.cum_turnover = p_data->m_total_value;		// notice this
	//p.avg_px = p_data->AveragePrice;	// notice this
	p.ask_px = p_data->m_ask_px;
	p.bid_px = p_data->m_bid_px;
	p.ask_vol = p_data->m_ask_share;
	p.bid_vol = p_data->m_bid_share;
	p.open_interest = p_data->m_total_pos.m_shfe;		// notice this
	memcpy(p.instr_str, p_data->m_symbol, sizeof(p_data->m_symbol));
	p.exch_time = (getSecondsFromClockStr(p_data->m_update_time) + CTimer::instance().getDayBeginTime()) * 1000000000L + p_data->m_millisecond * 1000000L;
	if(day_night_mode_ == MODE_NIGHT) {if(p_data->m_update_time[0] == '0') p.exch_time += 86400L * 1000000000L;}
	else {if(p_data->m_update_time[0] == '2') p.exch_time -= 86400L * 1000000000L;}
	md_writer_.commit();
}

static void getMcMd(const struct guava_udp_normal *p)
{
	CMDEngineEESOnload::instance_.processMd(p);
}
