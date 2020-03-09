#include "CMarketStruct.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "Logger.h"
#include <math.h>

CMarketStruct::CMarketStruct(){
    tradevol_ = 0;
    turnover_ = 0;
    last_cumturnover_ = 0;
    last_cumvol_ = 0;
    is_last_tick_ready_ = false;
    whichtick_ = 0;
    lastexchtime_ = 0;
    for(int count=0; count <5; count++){
        bidp_[count] = 0;
        bidv_[count] = 0;
        askp_[count] = 0;
        askv_[count] = 0;
        lastbidp_[count] = 0;
        lastbidv_[count] = 0;
        lastaskp_[count] = 0;
        lastaskv_[count] = 0;
    }
}

CMarketStruct::~CMarketStruct()
{}

bool CMarketStruct::init(const char *instr)
{
	uint32_t hash = INSTR_NAME_TO_HASH(instr);
	if(CTradeBaseInfo::instr_info_.find(hash) == CTradeBaseInfo::instr_info_.end())
	{
		LOG_ERR("cannot find instr: %s", instr);
		return false;
	}
    const tInstrumentInfo instr_info = CTradeBaseInfo::instr_info_[hash];
    exchange_ = instr_info.exch;
    ticksize_ = instr_info.tick_price;
    multiplier_ = instr_info.vol_multiple;
    return true;
}

void CMarketStruct::OnTick(const UnitedMarketData *md){
    bidp_[0] = int(round(md->bid_px/ticksize_));
    bidv_[0] = md->bid_vol;
    bidp_[1] = int(round(md->bid_px2/ticksize_));
    bidv_[1] = md->bid_vol2;
    bidp_[2] = int(round(md->bid_px3/ticksize_));
    bidv_[2] = md->bid_vol3;
    bidp_[3] = int(round(md->bid_px4/ticksize_));
    bidv_[3] = md->bid_vol4;
    bidp_[4] = int(round(md->bid_px5/ticksize_));
    bidv_[4] = md->bid_vol5;
    
    askp_[0] = int(round(md->ask_px/ticksize_));
    askv_[0] = md->ask_vol;
    askp_[1] = int(round(md->ask_px2/ticksize_));
    askv_[1] = md->ask_vol2;
    askp_[2] = int(round(md->ask_px3/ticksize_));
    askv_[2] = md->ask_vol3;
    askp_[3] = int(round(md->ask_px4/ticksize_));
    askv_[3] = md->ask_vol4;
    askp_[4] = int(round(md->ask_px5/ticksize_));
    askv_[4] = md->ask_vol5;
    
    exchtime_ = md-> exch_time/1000000000.0;
    is_last_tick_ready_ = is_last_tick_ready_? true : IsLastTickReady(md);    
    
    if(is_last_tick_ready_){    
        ++whichtick_;    
        tradevol_ = md->cum_vol - last_cumvol_;
        double turnover = md->cum_turnover - last_cumturnover_;
        turnover_ = int(round(turnover/(ticksize_*multiplier_)));
        if(exchange_ == EXCHANGEID_SHFE){
            tradevol_ = tradevol_/2;
            turnover_ = turnover_/2;
        }
        
        if (tradevol_ >0){
            avgprice_= (0.0 + turnover_)/tradevol_;
        }
        else{
            avgprice_ = 0.5*(askp_[0] + bidp_[0]);
        }
    }
    string timestamp = parseNano(exchtime_ * 1000000000, "%H:%M:%S") + "," + std::to_string(long(exchtime_ * 1000)%1000);
    LOG_LVL(DEBUGLEVEL0, "timestamp: %s, ask: %d, bid: %d, askv: %d, bidv: %d, avgp: %f, turnover: %f", timestamp.c_str(), askp_[0], bidp_[0], askv_[0], bidv_[0], avgprice_, turnover_);
                
}

bool CMarketStruct::IsLastTickReady(const UnitedMarketData *md){
    if(md->bid_px > 0 && md->cum_vol >0 && last_cumturnover_ > 0){
        return true;
    }
            
    return false;
}

void CMarketStruct::UpdateLastTickInfo(const UnitedMarketData *md){
    for(int count=0; count< LEVELCOUNT_; count++){
        lastaskp_[count] = askp_[count];
        lastaskv_[count] = askv_[count];
        lastbidp_[count] = bidp_[count];
        lastbidv_[count] = bidv_[count];
    }
    last_cumvol_ = md->cum_vol;
    last_cumturnover_ = md->cum_turnover;
    lastexchtime_ = exchtime_;
}

bool CMarketStruct::RetIsLastReady_() const{
    return is_last_tick_ready_;
}

bool CMarketStruct::RetIs2ndTick_() const{
    return (whichtick_ == 1);
}

int CMarketStruct::RetWhichTick_() const{
    return whichtick_;
}

int CMarketStruct::GetTradePriceRelatedVolume(int orderprice, int direction){
    int ordervolume = 0;
    if(LEVELCOUNT_ == 5){
        if(direction >0){
            for(int count =0; count <5; count++){
                if(orderprice <= bidp_[count]){
                    ordervolume = ordervolume + bidv_[count];
                }
                else if(orderprice >= askp_[count]){
                    ordervolume = ordervolume - askv_[count];
                }
                else
                    break;
            }
        }
        else{
            for(int count =0; count <5; count++){
                if(orderprice >= askp_[count]){
                    ordervolume = ordervolume + askv_[count];
                }
                else if(orderprice <= bidp_[count]){
                    ordervolume = ordervolume - bidv_[count];
                }
                else
                    break;
            }
        } 
    }
    else if(LEVELCOUNT_ == 1){         
        if (direction >0){
            if (orderprice <= bidp_[0]){
                ordervolume = (bidp_[0] - orderprice + 1) * bidv_[0];
            }            
            else if (orderprice >= askp_[0]){
                ordervolume = ordervolume - askv_[0] * (orderprice - askp_[0] + 1);
            }            
        }
        else{
            if (orderprice >= askp_[0]){
                ordervolume = (orderprice - askp_[0] + 1) * askv_[0];                
            }                        
            else if(orderprice <= bidp_[0]){
                ordervolume = ordervolume - bidv_[0] * (-orderprice + bidp_[0] + 1);
            }                 
        }            
    }   
    return ordervolume;
}

