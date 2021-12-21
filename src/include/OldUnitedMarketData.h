#ifndef OldUnitedMarketData_H_INCLUDED
#define OldUnitedMarketData_H_INCLUDED

#include <cstddef>

#include "float.h"

struct OldUnitedMarketData {
  int instr_idx = -1;
  float last_px = 0.0;
  int cum_vol = 0;
  double cum_turnover = 0.0;
  float avg_px = 0.0;
  float ask_px = 0.0;
  float bid_px = 0.0;
  int ask_vol = 0;
  int bid_vol = 0;
  double open_interest = 0.0;
  char instr_str[20] = {0};
  long exch_time = 0;
  float bid2 = 0.0;
  int bidvol2 = 0;
  float bid3 = 0.0;
  int bidvol3 = 0;
  float bid4 = 0.0;
  int bidvol4 = 0;
  float bid5 = 0.0;
  int bidvol5 = 0;
  float ask2 = 0.0;
  int askvol2 = 0;
  float ask3 = 0.0;
  int askvol3 = 0;
  float ask4 = 0.0;
  int askvol4 = 0;
  float ask5 = 0.0;
  int askvol5 = 0;
};

inline int getInstrIdx(const OldUnitedMarketData *p) { return p->instr_idx; }
inline float getLastPx(const OldUnitedMarketData *p) { return p->last_px; }
inline int getCumVol(const OldUnitedMarketData *p) { return p->cum_vol; }
inline double getCumTurnover(const OldUnitedMarketData *p) {
  return p->cum_turnover;
}
inline float getAvgPx(const OldUnitedMarketData *p) { return p->avg_px; }
inline float getAskPx(const OldUnitedMarketData *p) { return p->ask_px; }
inline float getBidPx(const OldUnitedMarketData *p) { return p->bid_px; }
inline int getAskVol(const OldUnitedMarketData *p) { return p->ask_vol; }
inline int getBidVol(const OldUnitedMarketData *p) { return p->bid_vol; }
inline long getExchTime(const OldUnitedMarketData *p) { return p->exch_time; }
inline double getOpenInt(const OldUnitedMarketData *p) {
  return p->open_interest;
}
inline const char *getInstrStr(const OldUnitedMarketData *p) {
  return p->instr_str;
}
inline float getBid2(const OldUnitedMarketData *p) { return p->bid2; }
inline float getBid3(const OldUnitedMarketData *p) { return p->bid3; }
inline float getBid4(const OldUnitedMarketData *p) { return p->bid4; }
inline float getBid5(const OldUnitedMarketData *p) { return p->bid5; }
inline float getAsk2(const OldUnitedMarketData *p) { return p->ask2; }
inline float getAsk3(const OldUnitedMarketData *p) { return p->ask3; }
inline float getAsk4(const OldUnitedMarketData *p) { return p->ask4; }
inline float getAsk5(const OldUnitedMarketData *p) { return p->ask5; }
inline int getBidVol2(const OldUnitedMarketData *p) { return p->bidvol2; }
inline int getBidVol3(const OldUnitedMarketData *p) { return p->bidvol3; }
inline int getBidVol4(const OldUnitedMarketData *p) { return p->bidvol4; }
inline int getBidVol5(const OldUnitedMarketData *p) { return p->bidvol5; }
inline int getAskVol2(const OldUnitedMarketData *p) { return p->askvol2; }
inline int getAskVol3(const OldUnitedMarketData *p) { return p->askvol3; }
inline int getAskVol4(const OldUnitedMarketData *p) { return p->askvol4; }
inline int getAskVol5(const OldUnitedMarketData *p) { return p->askvol5; }

struct StockMarketData_v20180420 {
  int instr_idx = -1;
  float last_px = 0.0;
  int cum_vol = 0;
  float cum_turnover = 0.0;
  float avg_px = 0.0;
  float ask_px = 0.0;
  float bid_px = 0.0;
  int ask_vol = 0;
  int bid_vol = 0;
  float open_interest = 0.0;
  char instr_str[20] = {0};
  long exch_time = 0;
  char date[16] = {0};
  char time[16] = {0};
  float bid2 = 0.0;
  int bidvol2 = 0;
  float bid3 = 0.0;
  int bidvol3 = 0;
  float bid4 = 0.0;
  int bidvol4 = 0;
  float bid5 = 0.0;
  int bidvol5 = 0;
  float ask2 = 0.0;
  int askvol2 = 0;
  float ask3 = 0.0;
  int askvol3 = 0;
  float ask4 = 0.0;
  int askvol4 = 0;
  float ask5 = 0.0;
  int askvol5 = 0;
};

struct OldUnitedMarketData_v20180223 {
  unsigned int instr_idx;
  float last_px;
  int last_vol;
  float turnover;
  float ask_px;
  float bid_px;
  int ask_vol;
  int bid_vol;
  float open_interest;
  char instr_str[20];
  long exch_time;
};

// still valid for v6.3.19 (checked on 2021.12.17)
struct CThostFtdcDepthMarketDataField_v638 {
  ///交易日
  char TradingDay[9];
  ///合约代码
  char InstrumentID[31];
  ///交易所代码
  char ExchangeID[9];
  ///合约在交易所的代码
  char ExchangeInstID[31];
  ///最新价
  double LastPrice;
  ///上次结算价
  double PreSettlementPrice;
  ///昨收盘
  double PreClosePrice;
  ///昨持仓量
  double PreOpenInterest;
  ///今开盘
  double OpenPrice;
  ///最高价
  double HighestPrice;
  ///最低价
  double LowestPrice;
  ///数量
  int Volume;
  ///成交金额
  double Turnover;
  ///持仓量
  double OpenInterest;
  ///今收盘
  double ClosePrice;
  ///本次结算价
  double SettlementPrice;
  ///涨停板价
  double UpperLimitPrice;
  ///跌停板价
  double LowerLimitPrice;
  ///昨虚实度
  double PreDelta;
  ///今虚实度
  double CurrDelta;
  ///最后修改时间
  char UpdateTime[9];
  ///最后修改毫秒
  int UpdateMillisec;
  ///申买价一
  double BidPrice1;
  ///申买量一
  int BidVolume1;
  ///申卖价一
  double AskPrice1;
  ///申卖量一
  int AskVolume1;
  ///申买价二
  double BidPrice2;
  ///申买量二
  int BidVolume2;
  ///申卖价二
  double AskPrice2;
  ///申卖量二
  int AskVolume2;
  ///申买价三
  double BidPrice3;
  ///申买量三
  int BidVolume3;
  ///申卖价三
  double AskPrice3;
  ///申卖量三
  int AskVolume3;
  ///申买价四
  double BidPrice4;
  ///申买量四
  int BidVolume4;
  ///申卖价四
  double AskPrice4;
  ///申卖量四
  int AskVolume4;
  ///申买价五
  double BidPrice5;
  ///申买量五
  int BidVolume5;
  ///申卖价五
  double AskPrice5;
  ///申卖量五
  int AskVolume5;
  ///当日均价
  double AveragePrice;
  ///业务日期
  char ActionDay[9];
};

struct EESMarketDepthQuoteData_v20135 {
  char TradingDay[9];         ///<交易日
  char InstrumentID[31];      ///<合约代码
  char ExchangeID[9];         ///<交易所代码
  char ExchangeInstID[31];    ///<合约在交易所的代码
  double LastPrice;           ///<最新价
  double PreSettlementPrice;  ///<上次结算价
  double PreClosePrice;       ///<昨收盘
  double PreOpenInterest;     ///<昨持仓量
  double OpenPrice;           ///<今开盘
  double HighestPrice;        ///<最高价
  double LowestPrice;         ///<最低价
  int Volume;                 ///<数量
  double Turnover;            ///<成交金额
  double OpenInterest;        ///<持仓量
  double ClosePrice;          ///<今收盘
  double SettlementPrice;     ///<本次结算价
  double UpperLimitPrice;     ///<涨停板价
  double LowerLimitPrice;     ///<跌停板价
  double PreDelta;            ///<昨虚实度
  double CurrDelta;           ///<今虚实度
  char UpdateTime[9];         ///<最后修改时间
  int UpdateMillisec;         ///<最后修改毫秒
  double BidPrice1;           ///<申买价一
  int BidVolume1;             ///<申买量一
  double AskPrice1;           ///<申卖价一
  int AskVolume1;             ///<申卖量一
  double BidPrice2;           ///<申买价二
  int BidVolume2;             ///<申买量二
  double AskPrice2;           ///<申卖价二
  int AskVolume2;             ///<申卖量二
  double BidPrice3;           ///<申买价三
  int BidVolume3;             ///<申买量三
  double AskPrice3;           ///<申卖价三
  int AskVolume3;             ///<申卖量三
  double BidPrice4;           ///<申买价四
  int BidVolume4;             ///<申买量四
  double AskPrice4;           ///<申卖价四
  int AskVolume4;             ///<申卖量四
  double BidPrice5;           ///<申买价五
  int BidVolume5;             ///<申买量五
  double AskPrice5;           ///<申卖价五
  int AskVolume5;             ///<申卖量五
  double AveragePrice;        ///<当日均价
} __attribute__((packed));

struct long_pos_v428 {
  int m_low;
  int m_high;
} __attribute__((packed));

union sl_pos_v428 {
  struct long_pos_v428 m_dce;
  double m_shfe;
} __attribute__((packed));

struct guava_udp_normal_v428 {
  unsigned int m_sequence;  ///<会话编号
  char m_exchange_id;       ///<市场  0 表示中金  1表示上期
  char m_channel_id;        ///<通道编号
  char m_quote_flag;        ///<行情标志  0 无time sale,无lev1,
  ///           1 有time sale,无lev1,
  ///           2 无time sale,有lev1,
  ///           3 有time sale,有lev1
  char m_symbol[8];       ///<合约
  char m_update_time[9];  ///<最后更新时间(秒)
  int m_millisecond;      ///<最后更新时间(毫秒)

  double m_last_px;               ///<最新价
  int m_last_share;               ///<最新成交量
  double m_total_value;           ///<成交金额
  union sl_pos_v428 m_total_pos;  ///<持仓量
  double m_bid_px;                ///<最新买价
  int m_bid_share;                ///<最新买量
  double m_ask_px;                ///<最新卖价
  int m_ask_share;                ///<最新卖量
} __attribute__((packed));

struct MarketDataHead {
  long local_time = 0;
};

struct MarketDataHead_v1 {
  long local_time;
};

#endif
