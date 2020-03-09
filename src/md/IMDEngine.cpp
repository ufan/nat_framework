/*
 * IMDEngine.cpp
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#include <time.h>
#include <memory>
#include "IMDEngine.h"
#include "CTimer.h"
#include "CSystemIO.h"
#include "IOCommon.h"
#include "ATSysUtils.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"

IMDEngine::IMDEngine()
{
	long begin = CTimer::instance().getDayBeginTime();
	long now = time(NULL);
	if(begin + 6 * 3600 <= now && now <= begin + 18 * 3600) // 6:00 ~ 18:00
	{
		day_night_mode_ = MODE_DAY;
	}
	else day_night_mode_ = MODE_NIGHT;
}

IMDEngine::~IMDEngine()
{

}

bool IMDEngine::initEngine(const json& j_conf)
{
	config_ = j_conf;
	if(!init())
	{
		ALERT("init engine err.");
		return false;
	}

	string write_path = string(IO_MDENGINE_BASE_PATH) + name_;
	if(!createPath(write_path))
	{
		ALERT("create md_io directory %s failed.", write_path.c_str());
		return false;
	}
	if(j_conf["MDEngine"].find("PageSize") != j_conf["MDEngine"].end())
	{
		long page_size = j_conf["/MDEngine/PageSize"_json_pointer];
		page_size *= MB;
		md_writer_.setPageSize(page_size);
	}
	if(!md_writer_.init(write_path + "/md"))
	{
		ALERT("init %s md writer err.", name_.c_str());
		return false;
	}

	self_id_ = (int)HASH_STR(name().c_str());
	if(!CSystemIO::instance().init())
	{
		ALERT("init system io writer err.");
		return false;
	}

	if(!getBaseInfo(j_conf["/TDEngine/engine_name"_json_pointer], j_conf["/TDEngine/timeout"_json_pointer]))
	{
		ALERT("getBaseInfo failed.");
		return false;
	}
	return true;
}

void IMDEngine::writeStartSignal()
{
	int cmd = IO_MD_START;
	md_writer_.write(&cmd, sizeof(cmd));
}

void IMDEngine::runEngine()
{
	ENGLOG("md_engine start...");

	// start read md thread
	if(!start())
	{
		release();
		ALERT("engine start failed.");
		return;
	}
	auto &j = config_["MDEngine"];
	if(j.find("subscribe") != j.end())
	{
		engine_subscribe(j["subscribe"]);
	}

	ENGLOG("md_engine start listening.");

	// start listen on system io
	unique_ptr<CRawIOReader> sys_reader(CSystemIO::instance().createReader());
	writeStartSignal();

	do_running_ = true;
	while(do_running_)
	{
		uint32_t len;
		tSysIOHead *p = (tSysIOHead*)sys_reader->read(len);
		if(p)
		{
			if(p->to == self_id_)
			{
				switch(p->cmd)
				{
				case IO_HEAT_BEAT:
				{
					tSysIOHead ack = {IO_HEAT_BEAT_ACK, p->source, self_id_, p->back_word};
					CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
					LOG_DBG("reply heatbeat to %d", p->source);
					break;
				}
				case IO_QUERY_SUBS_INSTR:
				{
					tSysIOHead ack = {IO_RSP_QUERY_SUBS_INSTR, p->source, self_id_, p->back_word};
					string data((const char*)&ack, sizeof(ack));
					json j_vec(querySubscribedInstrument());
					data += j_vec.dump();
					CSystemIO::instance().getWriter().write(data.c_str(), data.size() + 1);
					break;
				}
				case IO_SUBS_INSTR:
				{
					try
					{
						auto j = json::parse(p->data);
						vector<string> subs = j;
						engine_subscribe(subs);
						tSysIOHead ack = {IO_ACK, p->source, self_id_, p->back_word};
						CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
					}
					catch(...)
					{
						ALERT("json parse err: %s", p->data);
					}
					break;
				}
				case IO_UNSUBS_INSTR:
				{
					try
					{
						auto j = json::parse(p->data);
						vector<string> unsubs = j;
						engine_unsubscribe(unsubs);
						tSysIOHead ack = {IO_ACK, p->source, self_id_, p->back_word};
						CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
					}
					catch(...)
					{
						ALERT("json parse err: %s", p->data);
					}
					break;
				}
				case IO_TD_REQ_BASE_INFO:
				{
					string data = CTradeBaseInfo::toSysIOStruct(((tSysIOHead*)p)->source, self_id_, ((tSysIOHead*)p)->back_word);
					CSystemIO::instance().getWriter().write(data.data(), data.size());
					LOG_DBG("response base info query.");
					break;
				}
				}
			}
		}
		else
		{
			usleep(200000);		// 200ms
		}
	}

	release();
	//join();			// wait for finish complete

	ENGLOG("md_engine stopped.");
}

vector<string> IMDEngine::querySubscribedInstrument()
{
	vector<string> res;
	for(auto& kv : subs_instr_)
	{
		res.push_back(kv.first);
	}
	return res;
}

void IMDEngine::engine_subscribe(const vector<string> &instr)
{
	auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
	vector<string> real_subs;
	for(const auto& i : instr_set)
	{
		auto itr = subs_instr_.find(i);
		if(itr == subs_instr_.end())
		{
			real_subs.push_back(i);
			subs_instr_[i] = 1;
		}
		else
		{
			itr->second++;
		}
	}
	subscribe(real_subs);
}

void IMDEngine::engine_unsubscribe(const vector<string> &instr)
{
	auto instr_set = CTradeBaseInfo::productToInstrSet(instr);
	vector<string> real_unsubs;
	for(const auto& i : instr_set)
	{
		auto itr = subs_instr_.find(i);
		if(itr != subs_instr_.end())
		{
			if(--(itr->second) <= 0)
			{
				real_unsubs.push_back(i);
				subs_instr_.erase(i);
			}
		}
	}
	unsubscribe(real_unsubs);
}

bool IMDEngine::getBaseInfo(string td_engine_name, int timeout)
{
	int td_engine_id = (int)HASH_STR(td_engine_name.c_str());
	string res = sysRequest(IO_TD_REQ_BASE_INFO, IO_TD_RSP_BASE_INFO, td_engine_id, self_id_, timeout);
	if(res.size())
	{
		const tIOTDBaseInfo *p = (const tIOTDBaseInfo*)((const tSysIOHead*)res.data())->data;
		CTradeBaseInfo::set(p);
		md_writer_.write(res.data(), res.size());	// record this in md flow
		return true;
	}
	return false;
}
