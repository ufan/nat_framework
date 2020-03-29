/*
 * CWareHouseReader.cpp
 *
 *  Created on: 2018年8月2日
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
#include <limits.h>
#include <regex>
#include <glob.h>
#include "CWareHouseReader.h"
#include "IOCommon.h"
#include "Logger.h"
#include "CTimer.h"
#include "ATStructure.h"
#include "utils.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "OldUnitedMarketData.h"
using namespace std;

CWareHouseFileCursor::CWareHouseFileCursor(string file)
{
	file_ = file;
	fd_ = open(file.c_str(), O_RDONLY | O_CLOEXEC);
	if(fd_ < 0)
	{
		throw std::runtime_error(strerror(errno));
	}
	if(sizeof(file_head_) != read(fd_, &file_head_, sizeof(file_head_)))
	{
		cerr << file << " format err" << endl;
		throw std::runtime_error("file format err.");
	}
}
CWareHouseFileCursor::~CWareHouseFileCursor() {close();}
void CWareHouseFileCursor::close()
{
	if(fd_ >= 0)
	{
		::close(fd_);
		fd_ = -1;
	}
}
void CWareHouseFileCursor::getInstrumentInfo(vector<Instrument> &res)
{
	Instrument info;
	for(int i = 0; i < file_head_.inst_cnt; ++i)
	{
		int iret = read(fd_, &info, sizeof(info));
		if(iret < sizeof(info))
		{
			throw std::runtime_error("parse instrument info format err.");
		}
		res.push_back(info);
	}
}
int CWareHouseFileCursor::next()
{
	MarketDataHead md_head;
	int iread = read(fd_, &md_head, sizeof(md_head));
	if(iread == 0) return 0;
	else if(sizeof(md_head) != iread) return -1;
	local_time_ = md_head.local_time;

	CThostFtdcDepthMarketDataField_v638 data;
	iread = read(fd_, &data, sizeof(data));
	if(iread == 0) return 0;
	else if(sizeof(data) != iread) return -1;

	UnitedMarketData &umd = umd_.market_data;
	strncpy(umd.instr_str, data.InstrumentID, sizeof(umd.instr_str));
	umd.instr_str[sizeof(umd.instr_str) - 1] = '\0';
	umd.instr_hash = INSTR_NAME_TO_HASH(umd.instr_str);
	umd.last_px = data.LastPrice;
	umd.cum_vol = data.Volume;
	umd.cum_turnover = data.Turnover;
	umd.avg_px = data.AveragePrice;
	umd.ask_px = data.AskPrice1;
	umd.bid_px = data.BidPrice1;
	umd.ask_vol = data.AskVolume1;
	umd.bid_vol = data.BidVolume1;
	umd.open_interest = data.OpenInterest;
	umd.exch_time = (local_time_ + 8 * 3600000000000L) / 86400000000000L * 86400000000000L +
			getSecondsFromClockStr(data.UpdateTime) * 1000000000L + data.UpdateMillisec * 1000000L - 8 * 3600000000000L;
	if(umd.exch_time > local_time_ + 2 * 3600000000000L) umd.exch_time -= 86400000000000L;
	else if(umd.exch_time < local_time_ - 20 * 3600000000000L) umd.exch_time += 86400000000000L;
	return iread;
}

CWareHouseReader::CWareHouseReader()
{

}

CWareHouseReader::~CWareHouseReader()
{

}

void CWareHouseReader::openFile(string file)
{
	try
	{
		auto p = new CWareHouseFileCursor(file);
		files_.emplace_back(p);
	}
	catch(std::runtime_error &e)
	{
		LOG_ERR("open file %s err: %s", file.c_str(), e.what());
	}
}

void CWareHouseReader::switchDay()
{
	if(files_.empty()) return;
	cur_trading_day_ = files_[0]->file_head_.trading_date;
	for(int i = 1; i < files_.size(); ++i)
	{
		if(cur_trading_day_ > files_[i]->file_head_.trading_date)
		{
			cur_trading_day_ = files_[i]->file_head_.trading_date;
		}
	}
	// LOG_INFO("process trading day:%s", cur_trading_day_.c_str());
	CTradeBaseInfo::trading_day_ = cur_trading_day_;
	CTradeBaseInfo::instr_info_.clear();
	vector<Instrument> instrs;
	long min_nano = LONG_MAX;
	for(int i = 0; i < files_.size(); ++i)
	{
		if(cur_trading_day_ == files_[i]->file_head_.trading_date)
		{
			files_[i]->getInstrumentInfo(instrs);
			int ret = files_[i]->next();
			if(ret <= 0)
			{
				if(ret < 0)
				{
					LOG_ERR("detect file format err.");
				}
				files_[i]->close();
				files_.erase(files_.begin() + i);
				i--;
			}
			else if(min_nano > files_[i]->local_time_)
			{
				min_nano = files_[i]->local_time_;
			}
		}
	}
	for(auto &i : instrs)
	{
		uint32_t hash = INSTR_NAME_TO_HASH(i.instr_str);
		if(CTradeBaseInfo::instr_info_.count(hash) == 0)
		{
			tInstrumentInfo &info = CTradeBaseInfo::instr_info_[hash];
			info.instr_hash = hash;
			strncpy(info.instr, i.instr_str, sizeof(info.instr));
			info.instr[sizeof(info.instr) - 1] = '\0';
			info.exch = exchangeStr2int(i.exch_str);
			strncpy(info.product, i.prd_str, sizeof(info.product));
			info.product[sizeof(info.product) - 1] = '\0';
			info.product_hash = INSTR_NAME_TO_HASH(info.product);
			info.vol_multiple = i.volume_multiple;
			info.tick_price = i.price_tick;
			strncpy(info.expire_date, i.expire_date, sizeof(info.expire_date));
			info.expire_date[sizeof(info.expire_date) - 1] = '\0';
		}
	}
	CTradeBaseInfo::is_init_ = true;
	string data = CTradeBaseInfo::toSysIOStruct(0,0,0);
	if(do_set_time_) CTimer::instance().setTime(min_nano - 10000000000L);
	onSwitchDay(data);
}

int CWareHouseReader::merge(tIOMarketData &data_rtn)
{
	long local_time = LONG_MAX;
	int idx = -1;
	for(int i = 0; i < files_.size(); ++i)
	{
		if(cur_trading_day_ == files_[i]->file_head_.trading_date)
		{
			if(local_time > files_[i]->local_time_)
			{
				local_time = files_[i]->local_time_;
				idx = i;
			}
		}
	}
	if(idx >= 0)
	{
		local_time_ = local_time;
		if(do_set_time_ && local_time > CTimer::instance().getNano())
		{
			CTimer::instance().setTime(local_time);
		}
		memcpy(&data_rtn, &(files_[idx]->umd_), sizeof(tIOMarketData));
		int ret = files_[idx]->next();
		if(ret <= 0)
		{
			if(ret < 0)
			{
				LOG_ERR("detect file format err.");
			}
			files_[idx]->close();
			files_.erase(files_.begin() + idx);
		}
	}
	else
	{
		switchDay();
	}
	return idx;
}

void CWareHouseReader::loadFileList(string file_pattern, string start_day, string end_day)
{
	std::regex e("\\$\\{DAY\\}");
	for(string date = start_day; date <= end_day; date = calcDate(date, 1))
	{
		string f = std::regex_replace(file_pattern, e, date);
		try
		{
			openFile(f);
		}
		catch(std::runtime_error &e)
		{
			LOG_ERR("open file %s err: %s", file_pattern.c_str(), e.what());
		}
	}
}

void CWareHouseReader::loadFiles(vector<string> file_patterns, string start_day, string end_day)
{
	for(auto &i : file_patterns)
	{
		loadFileList(i, start_day, end_day);
	}
	switchDay();
	file_patterns_ = file_patterns;
	start_day_ = start_day;
	end_day_ = end_day;
}

const UnitedMarketData* CWareHouseReader::readTick()
{
	while(files_.size())
	{
		if(merge(tick_) >= 0)
		{
			return &(tick_.market_data);
		}
	}
	return nullptr;
}

void CWareHouseReader::setReadPos(long nano)
{
	clear();
	string start = parseNano(nano, "%Y%m%d");
	start = start < start_day_ ? start_day_ : start;
	loadFiles(file_patterns_, start, end_day_);

	int idx = -1;
	do
	{
		long local_time = LONG_MAX;
		idx = -1;
		for(int i = 0; i < files_.size(); ++i)
		{
			if(cur_trading_day_ == files_[i]->file_head_.trading_date)
			{
				if(local_time > files_[i]->local_time_)
				{
					local_time = files_[i]->local_time_;
					idx = i;
				}
			}
		}
		if(idx >= 0)
		{
			if(local_time >= nano) break;
			int ret = files_[idx]->next();
			if(ret <= 0)
			{
				if(ret < 0)
				{
					LOG_ERR("detect file format err.");
				}
				files_[idx]->close();
				files_.erase(files_.begin() + idx);
			}
		}
		else
		{
			switchDay();
			idx = 0;
		}
	}while(idx >= 0 && files_.size());
}


