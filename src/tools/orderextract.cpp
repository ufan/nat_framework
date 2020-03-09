/*
 * orderextract.cpp
 *
 *  Created on: 2018年3月12日
 *      Author: hongxu
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <iostream>
#include <stdio.h>
#include <map>
#include <fstream>
#include "ATStructure.h"
#include "IOCommon.h"
#include "CRawIOReader.h"
#include "CReaderPool.h"
#include "CTimer.h"
#include "CSystemIO.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class COrderExtractor
{
public:
	typedef struct _tOrderRecord
	{
		long 				order_nano = 0;
		tIOInputOrderField 	order;
		UnitedMarketData	tick;
		vector<tIOrderRtn>	rtns;
	} tOrderRecord;

public:
	COrderExtractor() {}
	virtual ~COrderExtractor()
	{
		if(output_.is_open())
		{
			output_.close();
		}
	}

public:
	void flush()
	{
		for(auto &kv : order_map_)
		{
			tOrderRecord &r = kv.second;
			if(r.order_nano > 0)
			{
				json o = json::object();
				o["order"] =
				{
					{"nano", r.order_nano},
					{"tick_nano", r.order.extra_nano},
					{"stg_name_hash", r.order.from},
					{"local_id", r.order.local_id},
					{"acc_idx", r.order.acc_idx},
					{"instr", string(r.order.instr)},
					{"instr_hash", r.order.instr_hash},
					{"price", r.order.price},
					{"vol", r.order.vol},
					{"dir", r.order.dir},
					{"off", r.order.off},
					{"stg_id", r.order.stg_id}
				};
				o["tick"] =
				{
					{"instr_hash", r.tick.instr_hash},
					{"last_px", r.tick.last_px},
					{"cum_vol", r.tick.cum_vol},
					{"cum_turnover", r.tick.cum_turnover},
					{"avg_px", r.tick.avg_px},
					{"ask_px", r.tick.ask_px},
					{"bid_px", r.tick.bid_px},
					{"ask_vol", r.tick.ask_vol},
					{"bid_vol", r.tick.bid_vol},
					{"ask_px2", r.tick.ask_px2},
					{"bid_px2", r.tick.bid_px2},
					{"ask_vol2", r.tick.ask_vol2},
					{"bid_vol2", r.tick.bid_vol2},
					{"ask_px3", r.tick.ask_px3},
					{"bid_px3", r.tick.bid_px3},
					{"ask_vol3", r.tick.ask_vol3},
					{"bid_vol3", r.tick.bid_vol3},
					{"ask_px4", r.tick.ask_px4},
					{"bid_px4", r.tick.bid_px4},
					{"ask_vol4", r.tick.ask_vol4},
					{"bid_vol4", r.tick.bid_vol4},
					{"ask_px5", r.tick.ask_px5},
					{"bid_px5", r.tick.bid_px5},
					{"ask_vol5", r.tick.ask_vol5},
					{"bid_vol5", r.tick.bid_vol5},
					{"open_interest", r.tick.open_interest},
					{"exch_time", r.tick.exch_time},
					{"instr_str", string(r.tick.instr_str)}
				};
				o["rtns"] = json::array();
				json &rtns = o["rtns"];
				for(auto &m : r.rtns)
				{
					rtns.push_back(
					{
						{"nano", *(long*)&m},
						{"msg_type", m.rtn_msg.msg_type},
						{"price", m.rtn_msg.price},
						{"vol", m.rtn_msg.vol},
						{"dir", m.rtn_msg.dir},
						{"off", m.rtn_msg.off},
						{"errid", m.rtn_msg.errid},
						{"msg", string(m.rtn_msg.msg)}
					});
				}
				output_ << o << endl;
			}
		}
		order_map_.clear();
	}

	bool init(string stg_path, string td_path, string md_path, long nano, string output)
	{
		output_.open(output, std::ofstream::out | std::ofstream::app);
		if(!output_)
		{
			LOG_ERR("open file %s failed.", output.c_str());
			return false;
		}

		if(not tick_reader_.init(md_path, nano))
		{
			LOG_ERR("add md path %s err.", md_path.c_str());
			return false;
		}

		unique_ptr<CRawIOReader> p_reader(new CRawIOReader);
		if(not p_reader->init(stg_path, nano))
		{
			LOG_ERR("add stg path %s err.", stg_path.c_str());
			return false;
		}
		reader_pool_.add(0, p_reader.release());

		unique_ptr<CRawIOReader> p_td_reader(new CRawIOReader);
		if(not p_td_reader->init(td_path, nano))
		{
			LOG_ERR("add td path %s err.", td_path.c_str());
			return false;
		}
		reader_pool_.add(1, p_td_reader.release());

		auto p_sys_reader = CSystemIO::instance().createReader();
		p_sys_reader->setReadPos(nano);
		reader_pool_.add(2, p_sys_reader);
		return true;
	}

	void run()
	{
		uint32_t len = 0;
		uint32_t hash = 0;
		while(const char* p = reader_pool_.seqRead(len, hash))
		{
			switch(hash)
			{
			case 0: // stg
			{
				if(*(int*)p == IO_SEND_ORDER)
				{
					tIOInputOrderField *pcmd = (tIOInputOrderField*)p;
					long k = ((long)pcmd->from) << 32 | pcmd->local_id;
					tOrderRecord &r = order_map_[k];
					r.order = *pcmd;
					r.order_nano = getIOFrameHead(p)->nano;
					if(pcmd->extra_nano > 0)
					{
						tick_reader_.setReadPos(pcmd->extra_nano);
						uint32_t l = 0;
						if(tIOMarketData* p_tick = (tIOMarketData*)tick_reader_.read(l))
						{
							r.tick = p_tick->market_data;
						}
					}
				}
				else if(*(int*)p == IO_ORDER_ACTION)
				{
					tIOrderAction *pcmd = (tIOrderAction*)p;
					long k = ((long)pcmd->from) << 32 | pcmd->local_id;
					auto itr = order_map_.find(k);
					if(itr != order_map_.end())
					{
						tIOrderRtn rtn;
						*(long*)&rtn = getIOFrameHead(p)->nano;
						rtn.rtn_msg.msg_type = ODS(CXLING);
						rtn.rtn_msg.local_id = pcmd->local_id;
						rtn.rtn_msg.instr_hash = pcmd->instr_hash;
						strcpy(rtn.rtn_msg.instr, pcmd->instr);
						itr->second.rtns.push_back(rtn);
					}
				}
				break;
			}
			case 1: // td
			{
				if(*(int*)p == IO_ORDER_RTN)
				{
					tIOrderRtn *pcmd = (tIOrderRtn*)p;
					long k = ((long)pcmd->to) << 32 | pcmd->rtn_msg.local_id;
					tIOrderRtn rtn;
					*(long*)&rtn = getIOFrameHead(p)->nano;
					rtn.rtn_msg = pcmd->rtn_msg;
					order_map_[k].rtns.push_back(rtn);
				}
				break;
			}
			case 2: // sys_reader
			{
				if(*(int*)p == IO_TD_RSP_BASE_INFO)
				{
					const char *p_trading_day = ((tIOTDBaseInfo*)(p + sizeof(tSysIOHead)))->trading_day;
					if(cur_trading_day_ != p_trading_day)
					{
						flush();
						cur_trading_day_ = p_trading_day;
						LOG_INFO("switch trading day %s", p_trading_day);
					}
				}
				break;
			}
			}
		}
		flush();
		output_.flush();
	}

protected:
	string						cur_trading_day_;
	map<long, tOrderRecord>		order_map_;
	CRawIOReader				tick_reader_;
	CReaderPool					reader_pool_;
	ofstream 					output_;
};


int main(int argc, char *argv[])
{
	if(argc != 6)
	{
		printf("Usage: %s stg_path td_path md_path start_time output_file\n", argv[0]);
		return -1;
	}

	initLogger("./logger.cnf");

	COrderExtractor extractor;

	long start_nano = parseTime(argv[4], "%Y%m%d-%H:%M:%S");
	ASSERT_RET(extractor.init(argv[1], argv[2], argv[3], start_nano, argv[5]), -1);
	extractor.run();

	return 0;
}

