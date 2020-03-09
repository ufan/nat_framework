#include "AccBase.h"
#include "CTradeBaseInfo.h"
#include "CTimer.h"
#include "MurmurHash2.h"
#include "ATConstants.h"
#include "SysConf.h"
#include "Logger.h"
#include <glob.h>
#include "utils.h"
#include "string.h"
#include <fstream>

bool AccBase::init(const char* name, const json& j)
{
	j_conf_ = j;
	strncpy(acc_name_, name, sizeof(acc_name_));
	return true;
}

bool AccBase::regInstr(const char* instr_name)
{
	uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
	auto itr = CTradeBaseInfo::instr_info_.find(instr_hash);
	if (itr == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %s in CTradeBaseInfo.", instr_name);
		return false;
	}
	tInstrumentInfo* p_instr_info = &(itr->second);
	uint32_t prd_hash = p_instr_info->product_hash;
	if (! isExistModPrd(prd_hash))
	{
		if (! regModPrd(p_instr_info->product))
		{
			ALERT("can't reg mod prd %s.", p_instr_info->product);
			return false;
		}
	}
	
	if (! isExistModInstr(instr_hash))
	{
		if (! regModInstr(instr_name))
		{
			ALERT("can't reg mod instr %s.", instr_name);
			return false;
		}
		map_instr_[instr_hash].p_prd = &map_prd_[prd_hash];
		map_instr_[instr_hash].p_acc = &mod_acc_;
	}
	
	return true;
}

bool AccBase::isExistModPrd(uint32_t prd_hash)
{
	return map_prd_.find(prd_hash) != map_prd_.end();
}

bool AccBase::isExistModInstr(uint32_t instr_hash)
{
	return map_instr_.find(instr_hash) != map_instr_.end();
}

bool AccBase::regModPrd(const char* prd_name)
{
	uint32_t prd_hash = INSTR_NAME_TO_HASH(prd_name);
	if (! isExistModPrd(prd_hash))
	{
		if (! map_prd_[prd_hash].init(prd_name))
		{
			ALERT("can't init prd %s.", prd_name);
			return false;
		}
	}
	
	return true;
}

bool AccBase::regModInstr(const char* instr_name)
{
	uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
	if (! isExistModInstr(instr_hash))
	{
		auto itr = CTradeBaseInfo::instr_info_.find(instr_hash);
		if (itr == CTradeBaseInfo::instr_info_.end())
		{
			ALERT("can't find %s in CTradeBaseInfo.", instr_name);
			return false;
		}
		tInstrumentInfo* p_instr_info = &(itr->second);
		if (! map_instr_[instr_hash].init(instr_name, p_instr_info->vol_multiple))
		{
			ALERT("can't init instr %s.", instr_name);
			return false;
		}
	}
	
	return true;
}

bool AccBase::getJsonFilePath(int t, string day, string &path)
{
	LOG_INFO("%s(%d, %s, %s), local_time: %ld, trading_day: %s", __FUNCTION__, t, day.c_str(), path.c_str(), CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str());

	string prefix;
	if(j_conf_.find("save_path") != j_conf_.end())
	{
		prefix = j_conf_["save_path"];
	}
	else
	{
		string stg_proc_name = string(acc_name_);
		size_t pos = stg_proc_name.find(SUB_ACCOUNT_FILE_SUFFIX);
		if(pos != string::npos)
		{
			stg_proc_name = stg_proc_name.substr(0, pos);
		}
		prefix = string(ACCOUNT_BASE_PATH) + stg_proc_name + "/";
	}
	if(t == 0 && !createPath(prefix))
	{
		LOG_ERR("create path %s err.", prefix.c_str());
		return false;
	}
	prefix += string(acc_name_) + ".";
	string today_path = prefix + day + ".json";

	if(t < 0)
	{
		glob_t buf;
		string pre = prefix + "[0-9]*.json";
		glob(pre.c_str(), 0, NULL, &buf);

		for(int i = buf.gl_pathc-1; i >= 0; i--)
		{
			string p = buf.gl_pathv[i];
			if(p < today_path)
			{
				path = p;
				break;
			}
		}
	    globfree(&buf);
	}
	else path = today_path;
	return true;
}

json AccBase::to_json()
{
	LOG_INFO("%s(), local_time: %ld, trading_day: %s", __FUNCTION__, CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str());

	json j, j_prd, j_instr;
	j["mod_acc"] = mod_acc_.to_json();
		
	for (auto it = map_prd_.begin(); it != map_prd_.end(); ++it)
	{
		ModPrd& mod_prd = it->second;
		UnitVol& unit_vol = mod_prd.unit_vol;
		if (   0 != unit_vol.pos_long_yd_ini
			|| 0 != unit_vol.pos_long_yd_cls_ing
			|| 0 != unit_vol.pos_long_td_opn_ing 
			|| 0 != unit_vol.pos_long_td_cls_ing
			|| 0 != unit_vol.pos_long
			|| 0 != unit_vol.pos_short_yd_ini
			|| 0 != unit_vol.pos_short_yd_cls_ing
			|| 0 != unit_vol.pos_short_td_opn_ing 
			|| 0 != unit_vol.pos_short_td_cls_ing
			|| 0 != unit_vol.pos_short
			)
		{
			j_prd[mod_prd.prd_name] = mod_prd.to_json();
		}
	}
	j["mod_prd"] = j_prd;
		
	for (auto it = map_instr_.begin(); it != map_instr_.end(); ++it)
	{
		ModInstr& mod_instr = it->second;
		UnitVol& unit_vol = mod_instr.unit_vol;
		if (   0 != unit_vol.pos_long_yd_ini
			|| 0 != unit_vol.pos_long_yd_cls_ing
			|| 0 != unit_vol.pos_long_td_opn_ing 
			|| 0 != unit_vol.pos_long_td_cls_ing
			|| 0 != unit_vol.pos_long
			|| 0 != unit_vol.pos_short_yd_ini
			|| 0 != unit_vol.pos_short_yd_cls_ing
			|| 0 != unit_vol.pos_short_td_opn_ing 
			|| 0 != unit_vol.pos_short_td_cls_ing
			|| 0 != unit_vol.pos_short
			)
		{
			j_instr[mod_instr.getInstrName()] = mod_instr.to_json();
		}
	}
	j["mod_instr"] = j_instr;

	return j;
}

bool AccBase::from_json(json& j)
{
	LOG_INFO("%s(json), local_time: %ld, trading_day: %s", __FUNCTION__, CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str());
	if (! mod_acc_.from_json(j["mod_acc"]))
	{
		ALERT("can't parse from json node mod_acc.");
		return false;
	}
	
	for (json::iterator it = j["mod_instr"].begin(); it != j["mod_instr"].end(); ++it)
	{
		if (! regInstr(it.key().c_str()) )
		{
			ALERT("can't reg instr %s.", it.key().c_str());
			return false;
		}
		uint32_t instr_hash = INSTR_NAME_TO_HASH(it.key().c_str());
		ModInstr& instr = map_instr_[instr_hash];
		json& j2 = it.value();
		if (! instr.from_json(j2))
		{
			ALERT("can't parse from json instr node: %s.", it.key().c_str());
			return false;
		}
	}
	
	for (json::iterator it = j["mod_prd"].begin(); it != j["mod_prd"].end(); ++it)
	{
		uint32_t prd_hash = INSTR_NAME_TO_HASH(it.key().c_str());
		ModPrd& prd = map_prd_[prd_hash];
		json& j2 = it.value();
		if (! prd.from_json(j2))
		{
			ALERT("can't parse from json prd node: %s.", it.key().c_str());
			return false;
		}
	}
	
	return true;
}

void AccBase::onSwitchDay()
{
	LOG_INFO("%s(), local_time: %ld, trading_day: %s", __FUNCTION__, CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str());

	mod_acc_.onSwitchDay();
	
	vector<uint32_t> vec_erase;
	for (auto it = map_prd_.begin(); it != map_prd_.end(); ++it)
	{
		ModPrd& mod_prd = it->second;
		UnitVol& unit_vol = mod_prd.unit_vol;
		if (0 == unit_vol.pos_long && 0 == unit_vol.pos_short)
		{
			vec_erase.push_back(it->first);
		}
		else
		{
			mod_prd.onSwitchDay();
		}
	}
	for (auto& hash : vec_erase)
	{
		map_prd_.erase(hash);
	}
	
	vec_erase.clear();
	for (auto it = map_instr_.begin(); it != map_instr_.end(); ++it)
	{
		ModInstr& mod_instr = it->second;
		UnitVol& unit_vol = mod_instr.unit_vol;
		if (0 == unit_vol.pos_long && 0 == unit_vol.pos_short)
		{
			vec_erase.push_back(it->first);
		}
		else
		{
			mod_instr.onSwitchDay();
		}
	}
	for (auto& hash : vec_erase)
	{
		map_instr_.erase(hash);
	}
}

bool AccBase::save(string day, bool force)
{
	LOG_INFO("%s(%s, %d), local_time: %ld, trading_day: %s", __FUNCTION__, day.c_str(), force, CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str());

	if(!force && j_conf_.find("save_account_daily") != j_conf_.end() && j_conf_["save_account_daily"].get<bool>() == false)
	{
		return true;
	}
	string path = day + ".json";
	if(not getJsonFilePath(0, day, path)) return false;
	ofstream file(path, ios::out);
	if (! file) 
	{
		ALERT("can't write file: %s.", path.c_str());
		return false;
	}
	
	json j = to_json();
	
	file << std::setw(4) << j << endl;
	file.close();
	return true;
}

bool AccBase::load()
{
	string path;
	if(not getJsonFilePath(-1, CTradeBaseInfo::trading_day_, path)) return false;
	if(path.empty()) return true;
	ifstream file(path);
	if (! file)
	{
		ALERT("can't read file: %s.", path.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	
	try 
	{
		json j = json::parse(content);
		if (! from_json(j))
		{
			ALERT("can't parse from json: %s.", content.c_str());
			return false;
		}
		
	}
	catch (...)
	{
		ALERT("can't parse json file: %s.", path.c_str());
		return false;
	}
	LOG_INFO("%s(), local_time: %ld, trading_day: %s, load acc file: %s", __FUNCTION__, CTimer::instance().getNano(), CTradeBaseInfo::trading_day_.c_str(), path.c_str());
	
	return true;
}

void AccBase::onOrderTrack(const tOrderTrack* p_ord_trk)
{
	auto itr = CTradeBaseInfo::instr_info_.find(p_ord_trk->instr_hash);
	if (itr == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %s in CTradeBaseInfo.", p_ord_trk->instr);
		return;
	}
	tInstrumentInfo* p_instr_info = &(itr->second);
	if (! regInstr(p_instr_info->instr))
	{
		ALERT("can't reg instr %s.", p_instr_info->instr);
		return;
	}
	ModInstr& mod_instr = map_instr_[p_ord_trk->instr_hash];
	
	if ( p_ord_trk->status & (ODS(SEND)|ODS(TDSEND)) )
	{
		LOG_DBG("onOrderTrack, New, [order_ref]%ld, [dir]%s, [off]%s, [px]%.2lf, [vol]%d", p_ord_trk->order_ref, getDirString(p_ord_trk->dir), getOffString(p_ord_trk->off), p_ord_trk->price, p_ord_trk->vol);
		mod_instr.onNew(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_ord_trk->vol);
	}
	if ( (p_ord_trk->status & ODS(CLOSED)) && (p_ord_trk->vol != p_ord_trk->vol_traded) )
	{
		LOG_DBG("onOrderTrack, Cxl, [order_ref]%ld, [dir]%s, [off]%s, [px]%.2lf, [vol]%d", p_ord_trk->order_ref, getDirString(p_ord_trk->dir), getOffString(p_ord_trk->off), p_ord_trk->price, p_ord_trk->vol - p_ord_trk->vol_traded);
		mod_instr.onCxl(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_ord_trk->vol - p_ord_trk->vol_traded);
	}
	if ( p_ord_trk->status & ODS(EXECUTION) )
	{
		double trd_px = (0 == p_ord_trk->vol_traded) ? 0.0 : p_ord_trk->amount_traded / p_ord_trk->vol_traded;
		LOG_DBG("onOrderTrack, Trd, [order_ref]%ld, [dir]%s, [off]%s, [px]%.2lf, [vol]%d, [vol_traded]%d, [trd_px]%.2lf, [trd_vol]%d", p_ord_trk->order_ref, getDirString(p_ord_trk->dir), getOffString(p_ord_trk->off), p_ord_trk->price, p_ord_trk->vol, p_ord_trk->vol_traded, trd_px, p_ord_trk->vol_traded);
		mod_instr.onTrd(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_ord_trk->vol, p_ord_trk->vol_traded, trd_px, p_ord_trk->vol_traded);
	}
}

void AccBase::onNew(int dir, int off, double px, int vol, uint32_t instr_hash)
{
	auto itr = CTradeBaseInfo::instr_info_.find(instr_hash);
	if (itr == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %u in CTradeBaseInfo.", instr_hash);
		return;
	}
	tInstrumentInfo* p_instr_info = &(itr->second);
	if (! regInstr(p_instr_info->instr))
	{
		ALERT("can't reg instr %s.", p_instr_info->instr);
		return;
	}
	ModInstr& mod_instr = map_instr_[instr_hash];
	LOG_DBG("onNew, [instr]%s, [dir]%s, [off]%s, [px]%.2lf, [vol]%d", p_instr_info->instr, getDirString(dir), getOffString(off), px, vol);
	mod_instr.onNew(dir, off, px, vol);
}

void AccBase::onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg)
{
	auto itr = CTradeBaseInfo::instr_info_.find(p_ord_trk->instr_hash);
	if (itr == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %s in CTradeBaseInfo.", p_ord_trk->instr);
		return;
	}
	tInstrumentInfo* p_instr_info = &(itr->second);
	if (! regInstr(p_instr_info->instr))
	{
		ALERT("can't reg instr %s.", p_instr_info->instr);
		return;
	}
	ModInstr& mod_instr = map_instr_[p_ord_trk->instr_hash];
	
	if ( p_rtn_msg->msg_type & (ODS(REJECT)|ODS(MARKET_REJECT)|ODS(CANCELED)) )
	{
		LOG_DBG("onRtn, Cxl, [instr]%s, [dir]%s, [off]%s, [px]%.2lf, [vol]%d", p_ord_trk->instr, getDirString(p_ord_trk->dir), getOffString(p_ord_trk->off), p_ord_trk->price, p_rtn_msg->vol);
		mod_instr.onCxl(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_rtn_msg->vol);
	}
	if ( p_rtn_msg->msg_type & ODS(EXECUTION) )
	{
		LOG_DBG("onRtn, Trd, [instr]%s, [dir]%s, [off]%s, [px]%.2lf, [vol]%d, [vol_traded]%d, [trd_px]%.2lf, [trd_vol]%d", p_ord_trk->instr, getDirString(p_ord_trk->dir), getOffString(p_ord_trk->off), p_ord_trk->price, p_ord_trk->vol, p_ord_trk->vol_traded, p_rtn_msg->price, p_rtn_msg->vol);
		mod_instr.onTrd(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_ord_trk->vol, p_ord_trk->vol_traded, p_rtn_msg->price, p_rtn_msg->vol);
	}
}

void AccBase::onTickPx(uint32_t instr_hash, double tick_px)
{
	auto itr = CTradeBaseInfo::instr_info_.find(instr_hash);
	if (itr == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %u in CTradeBaseInfo.", instr_hash);
		return;
	}
	tInstrumentInfo* p_instr_info = &(itr->second);
	if (! regInstr(p_instr_info->instr))
	{
		ALERT("can't reg instr %s.", p_instr_info->instr);
		return;
	}
	ModInstr& mod_instr = map_instr_[instr_hash];
	mod_instr.onTickPx(tick_px);
}

ModAcc* AccBase::getModAcc()
{
	return &mod_acc_;
}

ModInstr* AccBase::getModInstr(uint32_t instr_hash)
{
	auto itr = map_instr_.find(instr_hash);
	if (itr == map_instr_.end())
	{
		return nullptr;
	}
	else
	{
		return &(itr->second);
	}
}

ModPrd* AccBase::getModPrd(uint32_t prd_hash)
{
	auto itr = map_prd_.find(prd_hash);
	if (itr == map_prd_.end())
	{
		return nullptr;
	}
	else
	{
		return &(itr->second);
	}
}

vector<ModInstr> AccBase::getAllInstr()
{
	vector<ModInstr> vec_instr;
	for (auto& kv : map_instr_)
	{
		ModInstr& instr = kv.second;
		UnitVol& unit_vol = instr.unit_vol;
		if (0 != unit_vol.pos_long || 0 != unit_vol.pos_short
			|| 0 != unit_vol.pos_long_yd_cls_ing || 0 != unit_vol.pos_short_yd_cls_ing
			|| 0 != unit_vol.pos_long_td_opn_ing || 0 != unit_vol.pos_short_td_opn_ing
			|| 0 != unit_vol.pos_long_td_cls_ing || 0 != unit_vol.pos_short_td_cls_ing)
		{
			vec_instr.push_back(kv.second);
		}
	}
	return vec_instr;
}

vector<ModPrd> AccBase::getAllPrd()
{
	vector<ModPrd> vec_prd;
	for (auto& kv : map_prd_)
	{
		ModPrd& prd = kv.second;
		UnitVol& unit_vol = prd.unit_vol;
		if (0 != unit_vol.pos_long || 0 != unit_vol.pos_short
			|| 0 != unit_vol.pos_long_yd_cls_ing || 0 != unit_vol.pos_short_yd_cls_ing
			|| 0 != unit_vol.pos_long_td_opn_ing || 0 != unit_vol.pos_short_td_opn_ing
			|| 0 != unit_vol.pos_long_td_cls_ing || 0 != unit_vol.pos_short_td_cls_ing)
		{
			vec_prd.push_back(kv.second);
		}
	}
	return vec_prd;
}

UnitAmt* AccBase::getAccUnitAmt()
{
	return &mod_acc_.unit_amt;
}

UnitPnl* AccBase::getAccUnitPnl()
{
	return &mod_acc_.unit_pnl;
}

UnitVol* AccBase::getInstrUnitVol(uint32_t instr_hash)
{
	if (isExistModInstr(instr_hash))
	{
		return &map_instr_[instr_hash].unit_vol;
	}
	else
	{
		return nullptr;
	}
}

UnitAmt* AccBase::getInstrUnitAmt(uint32_t instr_hash)
{
	if (isExistModInstr(instr_hash))
	{
		return &map_instr_[instr_hash].unit_amt;
	}
	else
	{
		return nullptr;
	}
}

UnitPx* AccBase::getInstrUnitPx(uint32_t instr_hash)
{
	if (isExistModInstr(instr_hash))
	{
		return &map_instr_[instr_hash].unit_px;
	}
	else
	{
		return nullptr;
	}
}

UnitPnl* AccBase::getInstrUnitPnl(uint32_t instr_hash)
{
	if (isExistModInstr(instr_hash))
	{
		return &map_instr_[instr_hash].unit_pnl;
	}
	else
	{
		return nullptr;
	}
}

UnitVol* AccBase::getPrdUnitVol(uint32_t prd_hash)
{
	if (isExistModPrd(prd_hash))
	{
		return &map_prd_[prd_hash].unit_vol;
	}
	else
	{
		return nullptr;
	}
}

UnitAmt* AccBase::getPrdUnitAmt(uint32_t prd_hash)
{
	if (isExistModPrd(prd_hash))
	{
		return &map_prd_[prd_hash].unit_amt;
	}
	else
	{
		return nullptr;
	}
}

UnitPnl* AccBase::getPrdUnitPnl(uint32_t prd_hash)
{
	if (isExistModPrd(prd_hash))
	{
		return &map_prd_[prd_hash].unit_pnl;
	}
	else
	{
		return nullptr;
	}
}

