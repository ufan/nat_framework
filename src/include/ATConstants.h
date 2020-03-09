// copy from kungfu

#ifndef SRC_INCLUDE_ATCONSTANTS_H_
#define SRC_INCLUDE_ATCONSTANTS_H_

#include <stdint.h>

// Exchange names
#define EXCHANGE_SSE "SSE" //上海证券交易所
#define EXCHANGE_SZE "SZE" //深圳证券交易所
#define EXCHANGE_CFFEX "CFFEX" //中国金融期货交易所
#define EXCHANGE_SHFE "SHFE" //上海期货交易所
#define EXCHANGE_DCE "DCE" //大连商品交易所
#define EXCHANGE_CZCE "CZCE" //郑州商品交易所

///////////////////////////////////
// AtActionFlagType: 报单操作标志
///////////////////////////////////
//删除
#define AT_CHAR_Delete          '0'
//挂起
#define AT_CHAR_Suspend         '1'
//激活
#define AT_CHAR_Active          '2'
//修改
#define AT_CHAR_Modify          '3'

typedef char AtActionFlagType;

///////////////////////////////////
// AtContingentConditionType: 触发条件
///////////////////////////////////
//立即
#define AT_CHAR_Immediately     '1'
//止损
#define AT_CHAR_Touch           '2'
//止赢
#define AT_CHAR_TouchProfit     '3'
//预埋单
#define AT_CHAR_ParkedOrder     '4'
//最新价大于条件价
#define AT_CHAR_LastPriceGreaterThanStopPrice '5'
//最新价大于等于条件价
#define AT_CHAR_LastPriceGreaterEqualStopPrice '6'
//最新价小于条件价
#define AT_CHAR_LastPriceLesserThanStopPrice '7'
//最新价小于等于条件价
#define AT_CHAR_LastPriceLesserEqualStopPrice '8'
//卖一价大于条件价
#define AT_CHAR_AskPriceGreaterThanStopPrice '9'
//卖一价大于等于条件价
#define AT_CHAR_AskPriceGreaterEqualStopPrice 'A'
//卖一价小于条件价
#define AT_CHAR_AskPriceLesserThanStopPrice 'B'
//卖一价小于等于条件价
#define AT_CHAR_AskPriceLesserEqualStopPrice 'C'
//买一价大于条件价
#define AT_CHAR_BidPriceGreaterThanStopPrice 'D'
//买一价大于等于条件价
#define AT_CHAR_BidPriceGreaterEqualStopPrice 'E'
//买一价小于条件价
#define AT_CHAR_BidPriceLesserThanStopPrice 'F'
//买一价小于等于条件价
#define AT_CHAR_BidPriceLesserEqualStopPrice 'H'

typedef char AtContingentConditionType;

///////////////////////////////////
// AtDirectionType: 买卖方向
///////////////////////////////////
//买
#define AT_CHAR_Buy             '0'
//卖
#define AT_CHAR_Sell            '1'

typedef char AtDirectionType;

///////////////////////////////////
// AtForceCloseReasonType: 强平原因
///////////////////////////////////
//非强平
#define AT_CHAR_NotForceClose   '0'
//资金不足
#define AT_CHAR_LackDeposit     '1'
//客户超仓
#define AT_CHAR_ClientOverPositionLimit '2'
//会员超仓
#define AT_CHAR_MemberOverPositionLimit '3'
//持仓非整数倍
#define AT_CHAR_NotMultiple     '4'
//违规
#define AT_CHAR_Violation       '5'
//其它
#define AT_CHAR_Other           '6'
//自然人临近交割
#define AT_CHAR_PersonDeliv     '7'

typedef char AtForceCloseReasonType;

///////////////////////////////////
// AtHedgeFlagType: 投机套保标志
///////////////////////////////////
//投机
#define AT_CHAR_Speculation     '1'
//套利
#define AT_CHAR_Argitrage       '2'
//套保
#define AT_CHAR_Hedge           '3'
//做市商(femas)
#define AT_CHAR_MarketMaker     '4'
//匹配所有的值(femas)
#define AT_CHAR_AllValue        '9'

typedef char AtHedgeFlagType;

///////////////////////////////////
// AtOffsetFlagType: 开平标志
///////////////////////////////////
//开仓
#define AT_CHAR_Open            '0'
//平仓
#define AT_CHAR_Close           '1'
//强平
#define AT_CHAR_ForceClose      '2'
//平今
#define AT_CHAR_CloseToday      '3'
//平昨
#define AT_CHAR_CloseYesterday  '4'
//强减
#define AT_CHAR_ForceOff        '5'
//本地强平
#define AT_CHAR_LocalForceClose '6'
//不分开平
#define AT_CHAR_Non             'N'

#define AT_CHAR_Auto			'A'

typedef char AtOffsetFlagType;

///////////////////////////////////
// AtOrderPriceTypeType: 报单价格条件
///////////////////////////////////
//任意价
#define AT_CHAR_AnyPrice        '1'
//限价
#define AT_CHAR_LimitPrice      '2'
//最优价
#define AT_CHAR_BestPrice       '3'

typedef char AtOrderPriceTypeType;

///////////////////////////////////
// AtOrderStatusType: 报单状态
///////////////////////////////////
//全部成交（最终状态）
#define AT_CHAR_AllTraded       '0'
//部分成交还在队列中
#define AT_CHAR_PartTradedQueueing '1'
//部分成交不在队列中（部成部撤， 最终状态）
#define AT_CHAR_PartTradedNotQueueing '2'
//未成交还在队列中
#define AT_CHAR_NoTradeQueueing '3'
//未成交不在队列中（被拒绝，最终状态）
#define AT_CHAR_NoTradeNotQueueing '4'
//撤单
#define AT_CHAR_Canceled        '5'
//订单已报入交易所未应答
#define AT_CHAR_AcceptedNoReply '6'
//未知
#define AT_CHAR_Unknown         'a'
//尚未触发
#define AT_CHAR_NotTouched      'b'
//已触发
#define AT_CHAR_Touched         'c'
//废单错误（最终状态）
#define AT_CHAR_Error           'd'
//订单已写入
#define AT_CHAR_OrderInserted   'i'
//前置已接受
#define AT_CHAR_OrderAccepted   'j'

typedef char AtOrderStatusType;

///////////////////////////////////
// AtPosiDirectionType: 持仓多空方向
///////////////////////////////////
//净
#define AT_CHAR_Net             '1'
//多头
#define AT_CHAR_Long            '2'
//空头
#define AT_CHAR_Short           '3'

typedef char AtPosiDirectionType;

///////////////////////////////////
// AtPositionDateType: 持仓日期
///////////////////////////////////
//今日持仓
#define AT_CHAR_Today           '1'
//历史持仓
#define AT_CHAR_History         '2'
//两种持仓
#define AT_CHAR_Both            '3'

typedef char AtPositionDateType;

 
#define AT_CHAR_INIT			'1'
#define AT_CHAR_OPENED			'2'
#define AT_CHAR_OPENING			'3'
#define AT_CHAR_CLOSED			'4'
#define AT_CHAR_CLOSING			'5'
#define AT_CHAR_HOLD			'6'
#define AT_CHAR_HOLD_AVG_PRICE	'7'

typedef char AtPosiActionType;

///////////////////////////////////
// AtTimeConditionType: 有效期类型
///////////////////////////////////
//立即完成，否则撤销
#define AT_CHAR_IOC             '1'
//本节有效
#define AT_CHAR_GFS             '2'
//当日有效
#define AT_CHAR_GFD             '3'
//指定日期前有效
#define AT_CHAR_GTD             '4'
//撤销前有效
#define AT_CHAR_GTC             '5'
//集合竞价有效
#define AT_CHAR_GFA             '6'
//FAK或IOC(yisheng)
#define AT_CHAR_FAK             'A'
//FOK(yisheng)
#define AT_CHAR_FOK             'O'

typedef char AtTimeConditionType;

///////////////////////////////////
// AtVolumeConditionType: 成交量类型
///////////////////////////////////
//任何数量
#define AT_CHAR_AV              '1'
//最小数量
#define AT_CHAR_MV              '2'
//全部数量
#define AT_CHAR_CV              '3'

typedef char AtVolumeConditionType;

///////////////////////////////////
// AtYsHedgeFlagType: 易盛投机保值类型
///////////////////////////////////
//保值
#define AT_CHAR_YsB             'B'
//套利
#define AT_CHAR_YsL             'L'
//无
#define AT_CHAR_YsNon           'N'
//投机
#define AT_CHAR_YsT             'T'

typedef char AtYsHedgeFlagType;

///////////////////////////////////
// AtYsOrderStateType: 易盛委托状态类型
///////////////////////////////////
//终端提交
#define AT_CHAR_YsSubmit        '0'
//已受理
#define AT_CHAR_YsAccept        '1'
//策略待触发
#define AT_CHAR_YsTriggering    '2'
//交易所待触发
#define AT_CHAR_YsExctriggering '3'
//已排队
#define AT_CHAR_YsQueued        '4'
//部分成交
#define AT_CHAR_YsPartFinished  '5'
//完全成交
#define AT_CHAR_YsFinished      '6'
//待撤消(排队临时状态)
#define AT_CHAR_YsCanceling     '7'
//待修改(排队临时状态)
#define AT_CHAR_YsModifying     '8'
//完全撤单
#define AT_CHAR_YsCanceled      '9'
//已撤余单
#define AT_CHAR_YsLeftDeleted   'A'
//指令失败
#define AT_CHAR_YsFail          'B'
//策略删除
#define AT_CHAR_YsDeleted       'C'
//已挂起
#define AT_CHAR_YsSuppended     'D'
//到期删除
#define AT_CHAR_YsDeletedForExpire 'E'
//已生效——询价成功
#define AT_CHAR_YsEffect        'F'
//已申请——行权、弃权、套利等申请成功
#define AT_CHAR_YsApply         'G'

typedef char AtYsOrderStateType;

///////////////////////////////////
// AtYsOrderTypeType: 易盛委托类型
///////////////////////////////////
//市价
#define AT_CHAR_YsMarket        '1'
//限价
#define AT_CHAR_YsLimit         '2'

typedef char AtYsOrderTypeType;

///////////////////////////////////
// AtYsPositionEffectType: 易盛开平类型
///////////////////////////////////
//平仓
#define AT_CHAR_YsClose         'C'
//不分开平
#define AT_CHAR_YsNon           'N'
//开仓
#define AT_CHAR_YsOpen          'O'
//平当日
#define AT_CHAR_YsCloseToday    'T'

typedef char AtYsPositionEffectType;

///////////////////////////////////
// AtYsSideTypeType: 易盛买卖类型
///////////////////////////////////
//双边
#define AT_CHAR_YsAll           'A'
//买入
#define AT_CHAR_YsBuy           'B'
//无
#define AT_CHAR_YsNon           'N'
//卖出
#define AT_CHAR_YsSell          'S'

typedef char AtYsSideTypeType;

///////////////////////////////////
// AtYsTimeConditionType: 易盛委托有效类型
///////////////////////////////////
//当日有效
#define AT_CHAR_YsGFD           '0'
//撤销前有效
#define AT_CHAR_YsGTC           '1'
//指定日期前有效
#define AT_CHAR_YsGTD           '2'
//FAK或IOC
#define AT_CHAR_YsFAK           '3'
//FOK
#define AT_CHAR_YsFOK           '4'

typedef char AtYsTimeConditionType;

//////////////////////////////////
// 交易所宏定义 match ees
/////////////////////////////////
#define EXCHANGEID_SSE 		100 //上海证券交易所
#define EXCHANGEID_SZE  	101 //深圳证券交易所
#define EXCHANGEID_CFFEX 	102 //中国金融期货交易所
#define EXCHANGEID_SHFE 	103 //上海期货交易所
#define EXCHANGEID_DCE 		104 //大连商品交易所
#define EXCHANGEID_CZCE 	105 //郑州商品交易所
#define EXCHANGEID_INE		106 //能源中心
#define EXCHANGEID_SGE		107 //上海金交所


#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)

#define AT_INT_DAILY_CYCLE		-10
#define AT_INT_SESSION_CYCLE	-20

enum class emDir
{
	DIR_BUY = AT_CHAR_Buy,
	DIR_SELL = AT_CHAR_Sell,
};

enum class emOffset
{
	OPEN 				= AT_CHAR_Open,
	CLOSE 				= AT_CHAR_Close,
	FORCE_CLOSE 		= AT_CHAR_ForceClose,
	CLOSE_TD 			= AT_CHAR_CloseToday,
	CLOSE_YD 			= AT_CHAR_CloseYesterday,
	FORCE_OFF 			= AT_CHAR_ForceOff,
	LOCAL_FORCE_CLOSE 	= AT_CHAR_LocalForceClose,
	NON 				= AT_CHAR_Non,
	AUTO				= AT_CHAR_Auto,
};

// 回报类型
enum class emOrderRtnType
{
	NOT_SET				= 0,
	CLOSED				= 1,
	SEND				= (1 << 1),
	TDSEND				= (1 << 2),
	CXLING				= (1 << 3),
	ACCEPT				= (1 << 4),
	REJECT				= (1 << 5) | 1,
	MARKET_ACCEPT		= (1 << 6),
	MARKET_REJECT		= (1 << 7) | 1,
	EXECUTION			= (1 << 8),
	CANCEL_REJECT		= (1 << 9),
	CANCELED			= (1 << 10) | 1,
	ERR					= (1 << 11) | 1,
};
#define ODS(em_status) (static_cast<int>(emOrderRtnType::em_status))

// sendOrder 返回代码
enum class RetCode : int
{
	RET_OK 					= 0,
	RET_FAILED 				= -1,
	RET_RISK_CHECK_FAILED 	= -2,
};


#endif /* SRC_INCLUDE_ATCONSTANTS_H_ */
