// copy from kungfu

#ifndef SRC_INCLUDE_ATSTRUCTURE_H_
#define SRC_INCLUDE_ATSTRUCTURE_H_

#include <string>
#include <string.h>
#include "ATConstants.h"

typedef char char_19[19];
typedef char char_21[21];
typedef char char_64[64];
typedef char char_7[7];
typedef char char_9[9];
typedef char char_30[30];
typedef char char_31[31];
typedef char char_16[16];
typedef char char_13[13];
typedef char char_2[2];
typedef char char_11[11];

// 报单结构体
struct ATInputOrderField
{
	char_11                    	BrokerID;              //经纪公司代码
	char_16                    	UserID;                //用户代码
	char_19                    	InvestorID;            //投资者代码
	char_21                    	BusinessUnit;          //业务单元
	char_9                     	ExchangeID;            //交易所代码
	char_31                    	InstrumentID;          //合约代码
	char_21                    	OrderRef;              //报单引用
	double                     	LimitPrice;            //价格
	int                        	Volume;                //数量
	int                        	MinVolume;             //最小成交量
	AtTimeConditionType        	TimeCondition;         //有效期类型
	AtVolumeConditionType      	VolumeCondition;       //成交量类型
	AtOrderPriceTypeType       	OrderPriceType;        //报单价格条件
	AtDirectionType            	Direction;             //买卖方向
	AtOffsetFlagType           	OffsetFlag;            //开平标志
	AtHedgeFlagType            	HedgeFlag;             //投机套保标志
	AtForceCloseReasonType     	ForceCloseReason;      //强平原因
	double                     	StopPrice;             //止损价
	int                        	IsAutoSuspend;         //自动挂起标志
	AtContingentConditionType  	ContingentCondition;   //触发条件
	char_30                    	MiscInfo;              //委托自定义标签
};

// 回报结构体
struct ATRtnOrderField
{
	char_31                	InstrumentID;          //合约代码
	char_21                	OrderRef;              //报单引用
	char_11                	ExchangeID;            //交易所代码
	double                 	LimitPrice;            //价格
	int                    	VolumeTraded;          //今成交数量
	int                    	VolumeTotal;           //剩余数量
	int                    	VolumeTotalOriginal;   //数量
	AtTimeConditionType    	TimeCondition;         //有效期类型
	AtVolumeConditionType  	VolumeCondition;       //成交量类型
	AtOrderPriceTypeType   	OrderPriceType;        //报单价格条件
	AtDirectionType        	Direction;             //买卖方向
	AtOffsetFlagType       	OffsetFlag;            //开平标志
	AtHedgeFlagType        	HedgeFlag;             //投机套保标志
	AtOrderStatusType      	OrderStatus;           //报单状态
	int                    	RequestID;             //请求编号
};

// 成交回报
struct ATRtnTradeField
{
	char_11           	BrokerID;              //经纪公司代码
	char_16           	UserID;                //用户代码
	char_19           	InvestorID;            //投资者代码
	char_21           	BusinessUnit;          //业务单元
	char_31           	InstrumentID;          //合约代码
	char_21           	OrderRef;              //报单引用
	char_11           	ExchangeID;            //交易所代码
	char_21           	TradeID;               //成交编号
	char_31           	OrderSysID;            //报单编号
	char_11           	ParticipantID;         //会员代码
	char_21           	ClientID;              //客户代码
	double            	Price;                 //价格
	int               	Volume;                //数量
	char_13           	TradingDay;            //交易日
	char_13           	TradeTime;             //成交时间
	AtDirectionType   	Direction;             //买卖方向
	AtOffsetFlagType  	OffsetFlag;            //开平标志
	AtHedgeFlagType   	HedgeFlag;             //投机套保标志
};

// 撤单请求
struct ATOrderActionField
{
	char_11  	BrokerID;              //经纪公司代码
	char_19  	InvestorID;            //投资者代码
	char_31  	InstrumentID;          //合约代码
	char_11  	ExchangeID;            //交易所代码
	char_16  	UserID;                //用户代码
	char_21  	OrderRef;              //报单引用
	char_31  	OrderSysID;            //报单编号
	int      	RequestID;             //请求编号
	char     	ActionFlag;            //报单操作标志
	double   	LimitPrice;            //价格
	int      	VolumeChange;          //数量变化
	int      	KfOrderID;             //Kf系统内订单ID
};

struct UnitedMarketData
{
	uint32_t	instr_hash 		= 0;
	double 		last_px 		= 0.0;
	int 		cum_vol 		= 0;
	double		cum_turnover 	= 0.0;
	double 		avg_px 			= 0.0;
	double 		ask_px			= 0.0;
	double 		bid_px			= 0.0;
	int 		ask_vol			= 0;
	int 		bid_vol			= 0;
	double 		ask_px2			= 0.0;
	double 		bid_px2			= 0.0;
	int 		ask_vol2		= 0;
	int 		bid_vol2		= 0;
	double 		ask_px3			= 0.0;
	double 		bid_px3			= 0.0;
	int 		ask_vol3		= 0;
	int 		bid_vol3		= 0;
	double 		ask_px4			= 0.0;
	double 		bid_px4			= 0.0;
	int 		ask_vol4		= 0;
	int 		bid_vol4		= 0;
	double 		ask_px5			= 0.0;
	double 		bid_px5			= 0.0;
	int 		ask_vol5		= 0;
	int 		bid_vol5		= 0;
	double 		open_interest 	= 0.0;
	long 		exch_time 		= 0;
	char_31		instr_str 		= {0};

	std::string getInstrStr() {return instr_str;}
	void setInstrStr(std::string str)
	{strncpy(instr_str, str.c_str(), sizeof(instr_str)-1); instr_str[sizeof(instr_str)-1]='\0';}
};

struct Bar 
{
	uint32_t	instr_hash = -1;
	char		instr_str[31] = {0};
	int			cycle_sec = 0;
	long		bob = 0;
	long		eob = 0;
	double		open = 0.0;
	double		high = 0.0;
	double		low = 0.0;
	double		close = 0.0;
	double		delta_close = 0.0;
	int			cum_vol = 0;
	int			vol = 0;
	double		cum_turnover = 0.0;
	double		turnover = 0.0;
	double		open_int = 0.0;
	double		delta_open_int = 0.0;
	bool		is_auction = false;
	
	std::string getInstrStr() {return instr_str;}
};

#define INSTR_NAME_TO_HASH(inst_name)  MurmurHash2(inst_name, strlen(inst_name), 0x20180428)
#define INSTR_STR_TO_HASH(instr)  MurmurHash2(instr.c_str(), instr.size(), 0x20180428)

struct tOrderTrack
{
	int 			status 			= 0;
	uint32_t		instr_hash 		= 0;
	char_31			instr 			= {0};
	double			price 			= 0.0;
	int 			vol 			= 0;
	int				dir;
	int				off;
	int 			vol_traded 		= 0;
	double			amount_traded 	= 0.0;
	int 			from			= 0;	// send by whom
	int				local_id		= 0;	// id in sender
	int				acc_id			= 0;	// account id
	int 			stg_id 			= 0;
	long 			order_ref 		= -1;
	long			front_id		= -1;
	long 			session_id		= -1;

	std::string getInstrStr() {return instr;}
	void setInstrStr(std::string str)
	{strncpy(instr, str.c_str(), sizeof(instr)-1); instr[sizeof(instr)-1]='\0';}
};

struct tRtnMsg
{
	int				msg_type = 0;
	int				local_id;
	uint32_t		instr_hash;
	char_31 		instr;
	double			price;
	int				vol;
	int 			dir;
	int				off;
	long			order_ref;
	long			front_id;
	long 			session_id;
	int				errid 		= 0;
	char			msg[81] 	= {0};

	std::string getInstrStr() {return instr;}
	void setInstrStr(std::string str)
	{strncpy(instr, str.c_str(), sizeof(instr)-1); instr[sizeof(instr)-1]='\0';}

	std::string getMsg() {return msg;}
	void setMsg(std::string str)
	{strncpy(msg, str.c_str(), sizeof(msg)-1); msg[sizeof(msg)-1]='\0';}
};

struct tLastTick
{
	double 		ask_px			= 0.0;
	double 		bid_px			= 0.0;
	int 		ask_vol			= 0;
	int 		bid_vol			= 0;
	double 		ask_px2			= 0.0;
	double 		bid_px2			= 0.0;
	int 		ask_vol2		= 0;
	int 		bid_vol2		= 0;
	double 		ask_px3			= 0.0;
	double 		bid_px3			= 0.0;
	int 		ask_vol3		= 0;
	int 		bid_vol3		= 0;
	double 		ask_px4			= 0.0;
	double 		bid_px4			= 0.0;
	int 		ask_vol4		= 0;
	int 		bid_vol4		= 0;
	double 		ask_px5			= 0.0;
	double 		bid_px5			= 0.0;
	int 		ask_vol5		= 0;
	int 		bid_vol5		= 0;
	double		last_price		= 0.0;
	int			cum_vol			= 0;
	long		exch_time		= 0;
};

struct tInstrumentInfo
{
	uint32_t		instr_hash;
	char_31 		instr;
	int				exch;
	char_31			product;
	uint32_t		product_hash;
	int				vol_multiple;
	double 			tick_price;
	char_9			expire_date;
	bool			is_trading;

	std::string getInstrStr() {return instr;}
	void setInstrStr(std::string str)
	{strncpy(instr, str.c_str(), sizeof(instr)-1); instr[sizeof(instr)-1]='\0';}

	std::string getProduct() {return product;}
	void setProduct(std::string str)
	{strncpy(product, str.c_str(), sizeof(product)-1); product[sizeof(product)-1]='\0';}

	std::string getExpireDate() {return expire_date;}
	void setExpireDate(std::string str)
	{strncpy(expire_date, str.c_str(), sizeof(expire_date)-1); expire_date[sizeof(expire_date)-1]='\0';}
};

// 订阅的合约信息
struct tSubsInstrInfo
{
	tInstrumentInfo base_info;
	tLastTick		lst_tick;
};

#define MMAP_ORDER_TRACK_SIZE (256 * KB)  // must be power of 2
struct tOrderTrackMmap
{
	uint8_t			ver = 1;
	uint64_t		reserved;
	char			trading_day[12];
	tOrderTrack		order_track[MMAP_ORDER_TRACK_SIZE];
};

#define HASH_STR(str)  (MurmurHash2(str, strlen(str), 0x20180504))

#endif /* SRC_INCLUDE_ATSTRUCTURE_H_ */
