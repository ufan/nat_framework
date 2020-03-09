#include "IOMonitorHelper.h"
#include "CReaderPool.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <unordered_map>
#include "unistd.h"
#include "utils.h"

using namespace std;

uint32_t file_hash = 1;

const char* IOMonitorHelper::getCmdString(int cmd)
{
	switch (cmd)
	{
		case IO_UNKNOWN:
			return "IO_UNKNOWN";
		case IO_MARKET_DATA:
			return "IO_MARKET_DATA";
		case IO_MARKET_TRADING_DAY:
			return "IO_MARKET_TRADING_DAY";
		case IO_SIGNAL:
			return "IO_SIGNAL";
		case IO_SEND_ORDER:
			return "IO_SEND_ORDER";
		case IO_ORDER_ACTION:
			return "IO_ORDER_ACTION";
		case IO_ORDER_RTN:
			return "IO_ORDER_RTN";
		case IO_TD_START:
			return "IO_TD_START";
		case IO_TD_ADD_CLIENT:
			return "IO_TD_ADD_CLIENT";
		case IO_TD_REMOVE_CLIENT:
			return "IO_TD_REMOVE_CLIENT";
		case IO_TD_QUIT:
			return "IO_TD_QUIT";
		case IO_TD_ACK_ADD_CLIENT:
			return "IO_TD_ACK_ADD_CLIENT";
		case IO_TD_REQ_BASE_INFO:
			return "IO_TD_REQ_BASE_INFO";
		case IO_TD_RSP_BASE_INFO:
			return "IO_TD_RSP_BASE_INFO";
		case IO_TD_REQ_ORDER_TRACK:
			return "IO_TD_REQ_ORDER_TRACK";
		case IO_TD_RSP_ORDER_TRACK:
			return "IO_TD_RSP_ORDER_TRACK";
		case IO_HEAT_BEAT:
			return "IO_HEAT_BEAT";
		case IO_HEAT_BEAT_ACK:
			return "IO_HEAT_BEAT_ACK";
		case IO_MD_START:
			return "IO_MD_START";
		case IO_QUERY_SUBS_INSTR:
			return "IO_QUERY_SUBS_INSTR";
		case IO_RSP_QUERY_SUBS_INSTR:
			return "IO_RSP_QUERY_SUBS_INSTR";
		case IO_SUBS_INSTR:
			return "IO_SUBS_INSTR";
		case IO_UNSUBS_INSTR:
			return "IO_UNSUBS_INSTR";
		case IO_ACK:
			return "IO_ACK";
		case IO_SYSTEM_CMD:
			return "IO_SYSTEM_CMD";
		default:
			return "UNKNOWN CMD";
	}
}

void IOMonitorHelper::printBuf(const char *buf, uint32_t len)
{
	printf("data:{");
	for(uint32_t i = 0; i < len; i++)
	{
		if(isprint(buf[i]))
		{
			putchar(buf[i]);
		}
		else
		{
			printf("\\%02x", (unsigned char)buf[i]);
		}
	}
	printf("};");
}

void IOMonitorHelper::printSysIOHead(const char* p, uint32_t len)
{
	tSysIOHead* t = (tSysIOHead*)p;
	printf("[cmd]%s [to]%d [source]%d [back_word]%d",
		getCmdString(t->cmd),
		len < sizeof(tSysIOHead) ? 0 : t->to,
		len < sizeof(tSysIOHead) ? 0 : t->source,
		len < sizeof(tSysIOHead) ? 0 : t->back_word
		);
	size_t siz = sizeof(tSysIOHead);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOMarketData(const char* p, uint32_t len)
{
	tIOMarketData* t = (tIOMarketData*)p;
	printf("[cmd]%s [instr_hash]%u [last_px]%.2f [cum_vol]%d [cum_turnover]%.2lf [avg_px]%.2f \
[ask_px]%.2f [bid_px]%.2f [ask_vol]%d [bid_vol]%d [ask_px2]%.2f [bid_px2]%.2f [ask_vol2]%d [bid_vol2]%d \
[ask_px3]%.2f [bid_px3]%.2f [ask_vol3]%d [bid_vol3]%d [ask_px4]%.2f [bid_px4]%.2f [ask_vol4]%d [bid_vol4]%d \
[ask_px5]%.2f [bid_px5]%.2f [ask_vol5]%d [bid_vol5]%d [open_interest]%.2lf [exch_time]%ld [instr_str]%s",
		getCmdString(t->cmd),
		t->market_data.instr_hash,
		t->market_data.last_px,
		t->market_data.cum_vol,
		t->market_data.cum_turnover,
		t->market_data.avg_px,
		t->market_data.ask_px,
		t->market_data.bid_px,
		t->market_data.ask_vol,
		t->market_data.bid_vol,
		t->market_data.ask_px2,
		t->market_data.bid_px2,
		t->market_data.ask_vol2,
		t->market_data.bid_vol2,
		t->market_data.ask_px3,
		t->market_data.bid_px3,
		t->market_data.ask_vol3,
		t->market_data.bid_vol3,
		t->market_data.ask_px4,
		t->market_data.bid_px4,
		t->market_data.ask_vol4,
		t->market_data.bid_vol4,
		t->market_data.ask_px5,
		t->market_data.bid_px5,
		t->market_data.ask_vol5,
		t->market_data.bid_vol5,
		t->market_data.open_interest,
		t->market_data.exch_time,
		t->market_data.instr_str
		);
	size_t siz = sizeof(tIOMarketData);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOInputOrder(const char* p, uint32_t len)
{
	tIOInputOrderField* t = (tIOInputOrderField*)p;
	printf("[cmd]%s [extra_nano]%ld [from]%d [local_id]%d [acc_idx]%d [instr_hash]%u [instr]%s [price]%.2lf [vol]%d [dir]%s [off]%s",
		getCmdString(t->cmd),
		t->extra_nano,
		t->from,
		t->local_id,
		t->acc_idx,
		t->instr_hash,
		t->instr,
		t->price,
		t->vol,
		getDirString(t->dir),
		getOffString(t->off)
		);
	size_t siz = sizeof(tIOInputOrderField);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOrderAction(const char* p, uint32_t len)
{
	tIOrderAction* t = (tIOrderAction*)p;
	printf("[cmd]%s [from]%d [local_id]%d [acc_idx]%d [order_ref]%ld [front_id]%ld [session_id]%ld [instr_hash]%u [instr]%s",
		getCmdString(t->cmd),
		t->from,
		t->local_id,
		t->acc_idx,
		t->order_ref,
		t->front_id,
		t->session_id,
		t->instr_hash,
		t->instr
		);
	size_t siz = sizeof(tIOrderAction);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOrderRtn(const char* p, uint32_t len)
{
	tIOrderRtn* t = (tIOrderRtn*)p;
	printf("[cmd]%s [to]%d [msg_type]%s [local_id]%d [instr_hash]%u [instr]%s [price]%.2lf [vol]%d [dir]%s [off]%s \
[order_ref]%ld [front_id]%ld [session_id]%ld [errid]%d [msg]%s",
		getCmdString(t->cmd),
		t->to,
		getEmOrderRtnTypeString(t->rtn_msg.msg_type).c_str(),
		t->rtn_msg.local_id,
		t->rtn_msg.instr_hash,
		t->rtn_msg.instr,
		t->rtn_msg.price,
		t->rtn_msg.vol,
		getDirString(t->rtn_msg.dir),
		getOffString(t->rtn_msg.off),
		t->rtn_msg.order_ref,
		t->rtn_msg.front_id,
		t->rtn_msg.session_id,
		t->rtn_msg.errid,
		t->rtn_msg.msg
		);
	size_t siz = sizeof(tIOrderRtn);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOTDBaseInfo(const char* p, uint32_t len)
{
	printSysIOHead(p, sizeof(tSysIOHead));
	tIOTDBaseInfo* t = (tIOTDBaseInfo*)(p+sizeof(tSysIOHead));
	printf("[trading_day]%s [instr_cnt]%d ",
		t->trading_day,
		t->instr_cnt
		);
	
	printf("instrument_info:{");
	for (int i = 0; i < t->instr_cnt; ++i)
	{
		tInstrumentInfo* ot = (tInstrumentInfo*)t->instr;
		printf("(%d/%d)[instr_hash:%u;instr:%s;exch:%d;product:%s;product_hash:%u;vol_multiple:%d;\
tick_price:%.2lf;expire_date:%s;is_trading:%d],",
			i+1, t->instr_cnt,
			ot->instr_hash,
			ot->instr,
			ot->exch,
			ot->product,
			ot->product_hash,
			ot->vol_multiple,
			ot->tick_price,
			ot->expire_date,
			ot->is_trading
			);
		++ot;
	}
	printf("};");
	
	size_t siz = sizeof(tSysIOHead) + sizeof(tIOTDBaseInfo) + sizeof(tInstrumentInfo) * t->instr_cnt;
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOMArketTradingDay(const char* p, uint32_t len)
{
	tIOMArketTradingDay* t = (tIOMArketTradingDay*)p;
	printf("[cmd]%s [trading_day]%s [day_night_mode]%s",
		getCmdString(t->cmd),
		t->trading_day,
		t->day_night_mode
		);
	size_t siz = sizeof(tIOMArketTradingDay);
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printIOrderTrack(const char* p, uint32_t len)
{
	tIOrderTrack* t = (tIOrderTrack*)p;
	printf("[cmd]%s [to]%d [source]%d [back_word]%d [cnt]%d",
		getCmdString(t->cmd),
		t->to,
		t->source,
		t->back_word,
		t->cnt
		);
	
	printf("order_track:{");
	for (int i = 0; i < t->cnt; ++i)
	{
		tOrderTrack* ot = (tOrderTrack*)t->track;
		printf("(%d/%d)[[status]%s [instr_hash]%u [instr]%s [price]%.2lf [vol]%d [dir]%s [off]%s [vol_traded]%d \
[amount_traded]%.2lf [from]%d [local_id]%d [acc_id]%d [stg_id]%d [order_ref]%ld [front_id]%ld [session_id]%ld]],",
			i+1, t->cnt,
			getEmOrderRtnTypeString(ot->status).c_str(),
			ot->instr_hash,
			ot->instr,
			ot->price,
			ot->vol,
			getDirString(ot->dir),
			getOffString(ot->off),
			ot->vol_traded,
			ot->amount_traded,
			ot->from,
			ot->local_id,
			ot->acc_id,
			ot->stg_id,
			ot->order_ref,
			ot->front_id,
			ot->session_id
			);
		++ot;
	}
	printf("};");
	
	size_t siz = sizeof(tIOrderTrack) + sizeof(tOrderTrack) * t->cnt;
	if (len > siz)
	{
		printBuf(p + siz, len - siz);
	}
}

void IOMonitorHelper::printCmdContent(const char* p, uint32_t len)
{
	int cmd = *(int*)p;
	switch (cmd)
	{
		case IO_UNKNOWN:
		case IO_HEAT_BEAT:
		case IO_HEAT_BEAT_ACK:
		case IO_MD_START:
		case IO_QUERY_SUBS_INSTR:
		case IO_RSP_QUERY_SUBS_INSTR:
		case IO_SUBS_INSTR:
		case IO_UNSUBS_INSTR:
		case IO_ACK:
		case IO_SYSTEM_CMD:
		case IO_SIGNAL:
		case IO_TD_START:
		case IO_TD_ADD_CLIENT:
		case IO_TD_REMOVE_CLIENT:
		case IO_TD_QUIT:
		case IO_TD_ACK_ADD_CLIENT:
		case IO_TD_REQ_ORDER_TRACK:
		case IO_TD_REQ_BASE_INFO:
			printSysIOHead(p, len);
			break;
		case IO_MARKET_DATA:
			printIOMarketData(p, len);
			break;
		case IO_MARKET_TRADING_DAY:
			printIOMArketTradingDay(p, len);
			break;
		case IO_SEND_ORDER:
			printIOInputOrder(p, len);
			break;
		case IO_ORDER_ACTION:
			printIOrderAction(p, len);
			break;
		case IO_ORDER_RTN:
			printIOrderRtn(p, len);
			break;
		case IO_TD_RSP_BASE_INFO:
			printIOTDBaseInfo(p, len);
			break;
		case IO_TD_RSP_ORDER_TRACK:
			printIOrderTrack(p, len);
			break;
		default:
			break;
	}
}

string IOMonitorHelper::printOrderRtnContent(tRtnMsg* t)
{
	char ret[1024] = {0};
	snprintf(ret, sizeof(ret), "[msg_type]%s [local_id]%d [instr_hash]%u [instr]%s [price]%.2lf [vol]%d [dir]%s [off]%s \
[order_ref]%ld [front_id]%ld [session_id]%ld [errid]%d [msg]%s",
		getEmOrderRtnTypeString(t->msg_type).c_str(),
		t->local_id,
		t->instr_hash,
		t->instr,
		t->price,
		t->vol,
		getDirString(t->dir),
		getOffString(t->off),
		t->order_ref,
		t->front_id,
		t->session_id,
		t->errid,
		t->msg
		);
	return string(ret);
}

void IOMonitorHelper::readIO(const char *file, int from_no, long from_nano, long to_nano)
{
	vec_rtn.clear();
	CReaderPool pool;
	unordered_map<uint32_t, string> map_hash_file;
	uint32_t file_hash = 0;
	map_hash_file[file_hash] = string(file);
	pool.add(file_hash++, file, from_no, from_nano);
	const char *p = NULL;
	uint32_t len = 0;
	uint32_t hash = 0;
	while(true)
	{
		p = pool.seqRead(len, hash);
		if (p)
		{
			long nano = getIOFrameHead(p)->nano;
			if (from_nano <= nano && nano <= to_nano)
			{
				if (*(int*)p == IO_ORDER_RTN)
				{
					string t = parseNano(nano, "%Y%m%d %H:%M:%S");
//					LOG_DBG("[time_str]%s [time]%ld [length]%u [file]%s %s", t.c_str(), nano, len, map_hash_file[hash].c_str(), printOrderRtnContent(p, len).c_str());
					
					vec_rtn.emplace_back(((tIOrderRtn*)p)->rtn_msg);
				}
			}
		}
		else
		{
			usleep(50000);
			break;
		}
	}
}
