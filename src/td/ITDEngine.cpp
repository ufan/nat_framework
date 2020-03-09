/*
 * ITDEngine.cpp
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#include <fstream>
#include "CSystemIO.h"
#include "ITDEngine.h"
#include "utils.h"
#include "compiler.h"

atomic_flag ITDEngine::flag_ = ATOMIC_FLAG_INIT;

ITDEngine::ITDEngine()
{

}

ITDEngine::~ITDEngine()
{

}

bool ITDEngine::initEngine(const json& j_conf)
{
	auto &j_range = j_conf["/TDEngine/request_id_range"_json_pointer];
	request_id_start_ = j_range[0];
	request_id_end_ = j_range[1];
	request_id_ = request_id_start_;

	if(!init(j_conf))
	{
		ALERT("init engine err.");
		return false;
	}
	if(!getBaseInfo())
	{
		ALERT("getBaseInfo err.");
		return false;
	}
	if(!loadOrderTrack() || !updateOrderTrack())
	{
		ALERT("init order track err.");
		return false;
	}
	LOG_DBG("request_id_:%d", request_id_);

	if(!initAccountUtilis(j_conf))
	{
		ALERT("init account utilis err.");
		return false;
	}

	// init tdsend writer
	self_id_ = (int)HASH_STR(name().c_str());
	string io_dir = string(IO_TDENGINE_BASE_PATH) + name();
	if(!createPath(io_dir))
	{
		ALERT("create td_io directory %s failed.", io_dir.c_str());
		return false;
	}
	if(!writer_.init(io_dir + "/tdsend"))
	{
		ALERT("init writer err.");
		return false;
	}

	if(!CSystemIO::instance().init())
	{
		ALERT("init system io writer err.");
		return false;
	}
	// add system io reader
	read_pool_.add(0, CSystemIO::instance().createReader());	// 0 for system io
	return true;
}

bool ITDEngine::initAccountUtilis(const json& j_conf)
{
	int acc_cnt = getAccountCnt();
	auto& acc_conf = j_conf["Account"];
	for(int i = 0; i < acc_cnt; ++i)
	{
		acc_utilis_.emplace_back(new RiskTop);
		auto& acc = acc_utilis_.back();

		auto conf = j_conf["AccountDefault"];
		if(i < acc_conf.size()) conf = acc_conf[i];
		string acc_name = name() + string(".sub") + to_string(i);
		if(!acc->init(acc_name.c_str(), conf))
		{
			ALERT("init %d account position failed.", i);
			return false;
		}

		if(!acc->load())
		{
			ALERT("load last account err.");
			return false;
		}
		for(int i = request_id_start_; i < request_id_; i++)
		{
			acc->onOrderTrack(&get_request_track(i));
		}

		for(auto &kv : CTradeBaseInfo::instr_info_)
		{
			if(not acc->regInstr(kv.second.instr))
			{
				ALERT("account register instr %s err", kv.second.instr);
				return false;
			}
		}
	}
	return true;
}

bool ITDEngine::loadOrderTrack()
{
	if(otmmap_.load(name(), true))
	{
		tOrderTrackMmap *p = otmmap_.getBuf();
		p->ver = 1;
		p->reserved = 0;
		request_track_ = p->order_track;
		return true;
	}
	return false;
}

inline void ITDEngine::engine_req_order_action(const tIOrderAction* data)
{
	if((uint32_t)(data->acc_idx) < acc_utilis_.size())
	{
		req_order_action(data);
	}
	else
	{
		writeErrRtn(data, -1, "account idx err.");
	}
}

void ITDEngine::writeStartSignal()
{
	int cmd = IO_TD_START;
	writer_.write(&cmd, sizeof(cmd));
}

void ITDEngine::engine_on_close()
{
	for(auto &i : acc_utilis_)
	{
		i->save(CTradeBaseInfo::trading_day_, true);
	}
}

void ITDEngine::listening()
{
	ENGLOG("start listening...");
	writeStartSignal();

	do_running_ = true;
	while(do_running_)
	{
		uint32_t len = 0;
		uint32_t ioid;
		const char *p = read_pool_.seqRead(len, ioid);
		if(p)
		{
			if(0 != ioid)								// trader io
			{
				switch(*(int*)p)
				{
				case IO_SEND_ORDER:
				{
					req_order_insert((const tIOInputOrderField*)p);
					break;
				}
				case IO_ORDER_ACTION:
				{
					engine_req_order_action((const tIOrderAction*)p);
					break;
				}
				}
			}
			else if(((tSysIOHead*)p)->to == self_id_)	// system io
			{
				tSysIOHead *p_head = (tSysIOHead*)p;
				switch(p_head->cmd)
				{
				case IO_TD_ADD_CLIENT:
				{
					read_pool_.add(p_head->source, p_head->data, -1, -1);
					tSysIOHead ack = {IO_TD_ACK_ADD_CLIENT, p_head->source, self_id_, p_head->back_word};
					CSystemIO::instance().getWriter().write(&ack, sizeof(ack));
					LOG_DBG("add client %d:%s, reader_cnt:%d", p_head->source, p_head->data, read_pool_.size());
					break;
				}
				case IO_TD_REMOVE_CLIENT:
				{
					read_pool_.erase(p_head->source);
					LOG_DBG("remove client %d", p_head->source);
					break;
				}
				case IO_TD_QUIT:
				{
					do_running_ = false;
					break;
				}
				case IO_TD_REQ_BASE_INFO:
				{
					string data = CTradeBaseInfo::toSysIOStruct(p_head->source, self_id_, p_head->back_word);
					CSystemIO::instance().getWriter().write(data.data(), data.size());
					LOG_DBG("response base info query.");
					break;
				}
				case IO_TD_REQ_ORDER_TRACK:
				{
					rspOrderTrack(p_head);
					break;
				}

				case IO_USER_ADD_EXEC_ORDER:
				{
					tIOUserAddExecOrder *pcmd = (tIOUserAddExecOrder*)p;
					auto& util = acc_utilis_[pcmd->acc_idx];
					util->onNew(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol, pcmd->instr_hash, -1);
					if(auto pmod = util->getModInstr(pcmd->instr_hash))
					{
						pmod->onTrd(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol, pcmd->vol, pcmd->price, pcmd->vol);
					}
					break;
				}
				}
			}
		}
	}

	engine_on_close();
	ENGLOG("listen stopped.");
}

void ITDEngine::writeErrRtn(const tIOInputOrderField* data, int errid, const char* msg, int msgtp)
{
	tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
	p->cmd = IO_ORDER_RTN;
	p->to = data->from;
	p->rtn_msg.msg_type = msgtp;
	p->rtn_msg.local_id = data->local_id;
	p->rtn_msg.instr_hash = data->instr_hash;
	memcpy(p->rtn_msg.instr, data->instr, sizeof(p->rtn_msg.instr));
	p->rtn_msg.price = data->price;
	p->rtn_msg.vol = data->vol;
	p->rtn_msg.dir = data->dir;
	p->rtn_msg.off = data->off;
	p->rtn_msg.order_ref = -1;
	p->rtn_msg.front_id = -1;
	p->rtn_msg.session_id = -1;
	p->rtn_msg.errid = errid;
	strncpy(p->rtn_msg.msg, msg, sizeof(p->rtn_msg.msg));
	p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
	writer_.commit();
}

void ITDEngine::writeErrRtn(const tIOrderAction* data, int errid, const char* msg, int msgtp)
{
	tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
	p->cmd = IO_ORDER_RTN;
	p->to = data->from;
	p->rtn_msg.msg_type = msgtp;
	p->rtn_msg.local_id = data->local_id;
	p->rtn_msg.instr_hash = data->instr_hash;
	memcpy(p->rtn_msg.instr, data->instr, sizeof(p->rtn_msg.instr));
	p->rtn_msg.order_ref = data->order_ref;
	p->rtn_msg.front_id = data->front_id;
	p->rtn_msg.session_id = data->session_id;
	p->rtn_msg.errid = errid;
	strncpy(p->rtn_msg.msg, msg, sizeof(p->rtn_msg.msg));
	p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
	writer_.commit();
}

tIOrderRtn* ITDEngine::writeRtnFromTrack(tOrderTrack &request_track)
{
	tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
	p->cmd = IO_ORDER_RTN;
	p->to = request_track.from;
	p->rtn_msg.local_id = request_track.local_id;
	strncpy(p->rtn_msg.instr, request_track.instr, sizeof(p->rtn_msg.instr)-1);
	p->rtn_msg.instr[sizeof(p->rtn_msg.instr)-1] = '\0';
	p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
	p->rtn_msg.price = request_track.price;
	p->rtn_msg.vol = request_track.vol;
	p->rtn_msg.dir = request_track.dir;
	p->rtn_msg.off = request_track.off;
	p->rtn_msg.order_ref = request_track.order_ref;
	p->rtn_msg.front_id = request_track.front_id;
	p->rtn_msg.session_id = request_track.session_id;
	return p;
}

void ITDEngine::rspOrderTrack(tSysIOHead *req)
{
	string data(sizeof(tIOrderTrack), 0);
	int cnt = 0;
	for(int i = request_id_start_; i < request_id_; i++) // start from 1
	{
		 tOrderTrack& ot = get_request_track(i);
		 if(ot.from == req->source)
		 {
			 data += string((const char*)&ot, sizeof(ot));
			 cnt ++;
		 }
	}
	tIOrderTrack *head = (tIOrderTrack*)data.data();
	head->cmd = IO_TD_RSP_ORDER_TRACK;
	head->to = req->source;
	head->source = self_id_;
	head->back_word = req->back_word;
	head->cnt = cnt;
	CSystemIO::instance().getWriter().write(data.data(), data.size());
	LOG_DBG("response order track query to %d, order cnt:%d.", head->to, head->cnt);
}

