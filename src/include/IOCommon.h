/*
 * IOCommon.h
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#ifndef SRC_INCLUDE_IOCOMMON_H_
#define SRC_INCLUDE_IOCOMMON_H_

#include "ATStructure.h"
#include "ATConstants.h"

enum emIOCmdType
{
	IO_UNKNOWN,

	IO_MARKET_DATA,				// 市场行情
	IO_MARKET_TRADING_DAY,		// trading day

	IO_SIGNAL,					// 信号类型

	IO_SEND_ORDER,				// 发单
	IO_ORDER_ACTION,			// delete order
	IO_ORDER_RTN,				// 回报

	IO_TD_START,				// TDEngine 启动
	IO_TD_ADD_CLIENT,			// TDEngine 增加客户监听
	IO_TD_REMOVE_CLIENT,		// TDEngine 移除客户监听
	IO_TD_QUIT,					// TDEngine 退出
	IO_TD_ACK_ADD_CLIENT,		// TDEngine 响应增加客户
	IO_TD_REQ_BASE_INFO,		// TDEngine 查询基础信息
	IO_TD_RSP_BASE_INFO,		// TDEngine 响应查询基础信息
	IO_TD_REQ_ORDER_TRACK,		// TDEngine 查询 order track
	IO_TD_RSP_ORDER_TRACK,		// TDEngine 响应 order track 查询

	IO_HEAT_BEAT,				// 心跳包
	IO_HEAT_BEAT_ACK,			// 心跳包回应

	IO_MD_START,				// MDEngine start
	IO_QUERY_SUBS_INSTR,		// 查询订阅的合约
	IO_RSP_QUERY_SUBS_INSTR,	// 回应查询订阅合约
	IO_SUBS_INSTR,				// 订阅合约
	IO_UNSUBS_INSTR,			// 取消订阅合约

	IO_ACK,						// ack响应

	IO_SYSTEM_CMD,				// 系统命令
	IO_USER_CMD,				// user command

	IO_USER_ADD_EXEC_ORDER,		// add execution order by manually
};

// pack 1
#pragma pack(1)
struct tSysIOHead
{
	int				cmd;			// cmd type
	int				to;
	int				source;
	int				back_word;		// 回带字段
	char			data[0]; // extention of GNU C, zero-length array for holding variable-length content
};

struct tIOMarketData
{
	int 					cmd = IO_MARKET_DATA;
	UnitedMarketData		market_data;
};

struct tIOInputOrderField
{
	int 		cmd = IO_SEND_ORDER;
	long 		extra_nano;
	int			from;
	int			local_id;
	int			acc_idx;
	uint32_t	instr_hash;
	char_31		instr;
	double 		price;
	int 		vol;
	uint8_t		dir;
	uint8_t		off;
	int 		stg_id;
};

struct tIOrderAction
{
	int 		cmd = IO_ORDER_ACTION;
	int 		from;
	int 		local_id;
	int			acc_idx;
	long 		order_ref;
	long		front_id;
	long 		session_id;
	uint32_t	instr_hash;
	char_31 	instr;
};

struct tIOrderRtn
{
	int				cmd = IO_ORDER_RTN;
	int				to;
	tRtnMsg			rtn_msg;
};

struct tIOTDBaseInfo
{
	char			trading_day[12];
	int				instr_cnt;
	tInstrumentInfo	instr[0];
};

struct tIOMArketTradingDay
{
	int 		cmd = IO_MARKET_TRADING_DAY;
	char		trading_day[12];
	char		day_night_mode[8];
};

struct tIOrderTrack : tSysIOHead
{
	int cnt = 0;
	tOrderTrack track[0];
};

struct tIOUserAddExecOrder : tSysIOHead
{
	uint32_t  	instr_hash;
	uint8_t		dir;
	int			off;
	double		price;
	int			vol;
	int			acc_idx;
	int			stg_id;
};

#pragma pack()


#endif /* SRC_INCLUDE_IOCOMMON_H_ */
