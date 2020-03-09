#ifndef ISTATUS_H_INCLUDED
#define ISTATUS_H_INCLUDED

enum LogThreadType {
	LogType_MD,
	LogType_TD,
	LogType_ENG
};

enum LogStructType {
	CTP_Md,
	Xele_Md,
	Ord_Trk,
	Rtn_Msg
};

enum DeskType {
	DT_Sim,
	DT_CTP,
	DT_CTPMini,
	DT_Xele,
	DT_SimStk
};

enum UnitedMarketDataType {
	MDT_CTP_v638,
	MDT_Xele_SHFE,
	MDT_Xele_Not_SHFE,
	MDT_EES_v20135,
	MDT_EES_sf_v428
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
	TRADE_QRY_POSITION,
	TRADE_QRY_ACCOUNT,
	TRADE_QUERY_INSTRUMENTS
};

enum IndexStatus {
	Idx_NULL,
	Idx_Ready
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
	OST_NULL,
	OST_PendingNew,
	OST_RejectByDesk,
	OST_RejectByExchange,
	OST_AcceptByDesk,
	OST_NoTradeQueueing,
	OST_NoTradeNotQueueing,
	OST_PartTradedQueueing,
	OST_PartTradedNotQueueing,
	OST_CancelByExchange,
	OST_AllTraded,
	OST_PendingDelete,
	OST_RejectDeleteByDesk,
	OST_RejectDeleteByExchange
};

enum OMSDirection {
    OD_Buy='0',
    OD_Sell='1'
};

enum OMSOffset {
    OO_Open='0',
    OO_Close='1',
    OO_CloseToday='3'
};

enum ProductClassType {
	PC_Futures='1',
	PC_Options='2',
	PC_Combination='3',
	PC_Spot='4',
	PC_EFP='5',
	PC_SpotOption='6'
};

enum OptionsTypeType {
	OT_CallOptions='1',
	OT_PutOptions='2'
};

enum CombinationTypeType {
	CT_Future='0',
	CT_Bul='1',
	CT_Ber='2',
	CT_Std='3',
	CT_Stg='4',
	CT_Prt='5',
	CT_Cld='6'
};

enum MaxMarginSideAlgorithmType {
	MMSA_No='0',
	MMSA_Yes='1'
};

enum RtnMsgType {
	RM_Rtn,
	RM_Trd
};

#endif // ISTATUS_H_INCLUDED
