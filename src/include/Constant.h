#ifndef CONSTANT_H
#define CONSTANT_H

#define MDENGINE  "MdEngine"

enum LogThreadType {
	LTT_MD,
	LTT_TD,
	LTT_ENG
};

enum LogStructType {
	LST_MdHeadAndUmd=0,
	LST_SendOrder=1,
	LST_DeleteOrder=2,
	LST_RtnMsg=3
};

enum DeskType {
	DT_Sim=0,
	DT_CTP=1,
	DT_CTPMini=2,
	DT_EES=3,
	DT_Xele=4
};

// don't change value!!
enum UnitedMarketDataType {
	Uni_v20180223=1,
	Uni_v20180415=2,
	CTP_v638=101,
	EES_v20135=201,
	EES_sf_v428=301,
	Xele_SHFE=401,
	Xele_Not_SHFE=501,
	Stock_v20180420=601
};

enum MdHandlerStatus {
	MD_INIT,
	MD_LOGIN,
	MD_QUEUE_INIT,
	MD_SUBSCRIBE
};

enum TradeHandlerStatus {
	TRADE_INIT,
	TRADE_CONNECTED,
	TRADE_LOGIN,
	TRADE_SETTLEMENT_INFO_CONFIRM,
	TRADE_QRY_ACCOUNT,
	TRADE_QRY_POSITION,
	TRADE_QRY_MARGIN,
	TRADE_QRY_COMMISSION,
	TRADE_QRY_ORDER,
	TRADE_QRY_TRADE,
	TRADE_QUERY_INSTRUMENTS
};

enum OMSInstrumentStatus {
	IS_BeforeTrading='0',
	IS_NoTrading='1',
	IS_Continous='2',
	IS_AuctionOrdering='3',
	IS_AuctionBalance='4',
	IS_AuctionMatch='5',
	IS_Closed='6'
};

enum OMSOrderStatus {
	OS_NULL=0,
	OS_PendingNew=1,
	OS_Reject=2,
	OS_Accept=3,
	OS_MarketReject=4,
	OS_MarketAccept=5,
	OS_OpendPartTraded=6,
	OS_ClosedNoTraded=7,
	OS_ClosedPartTraded=8,
	OS_ClosedAllTraded=9,
	OS_PendingDelete=10
};

enum OMSDirection {
	OD_Buy='0',
	OD_Sell='1'
};

enum OMSOffset {
	OO_Auto='a',
	OO_Open='0',
	OO_Close='1',
	OO_CloseToday='3',
	OO_CloseYesterday='4'
};

enum ProductClassType {
	PC_Futures='1',
	PC_Options='2',
	PC_Combination='3',
	PC_Spot='4',
	PC_EFP='5',
	PC_SpotOption='6'
};

enum OptionsType {
	OT_Call='1',
	OT_Put='2'
};

enum OMSPosiType {
	PT_Closed=5,
	PT_Closing=6,
	PT_Position=9,
	PT_YdInit=10,
	PT_YdClosed=15,
	PT_YdClosing=16,
	PT_YdPosition=19,
	PT_TdOpened=20,
	PT_TdOpening=21,
	PT_TdClosed=25,
	PT_TdClosing=26,
	PT_TdPosition=29,
	PT_AvgPrice=40
};

enum OMSPosiDirection {
	PD_Net='1',
	PD_Long='2',
	PD_Short='3'
};

// don't change value!!
enum RtnMsgType {
	RM_Unknown=0,
	RM_SendOrder=1,
	RM_OrderReject=2,
	RM_OrderAccept=3,
	RM_OrderMarketReject=4,
	RM_OrderMarketAccept=5,
	RM_OrderExecution=6,
	RM_DeleteOrder=7,
	RM_CxlOrderReject=8,
	RM_OrderCxled=9
};

#endif