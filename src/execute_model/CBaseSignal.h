// design the HFT signal sets 
#ifndef CBASESIGNAL_H_
#define CBASESIGNAL_H_
#include "PublicFun.h"
#include "CMarketStruct.h"

class CBaseSignal
{   
public:
    double skewalpha_;
    double composite_askvol_; //composite 
    double composite_bidvol_;
    double composite_logskew_;
    
    double realtradeaskvol_;
    double realtradebidvol_;
    
    double bookbid_;
    double bookask_;
    
    double timediff_;    
    double horizons_[TOTAL_HORIZON_];
    
    double trade_[TOTAL_HORIZON_];// fixed length
    double book_[TOTAL_HORIZON_];
    double CR_[TOTAL_HORIZON_];
    double OBD_[TOTAL_HORIZON_]; // orderbookdynamics
    
    double OBD_alpha_;
    
    int    delta_md_Bid_[10];
    int    delta_md_Ask_[10];
    int    delta_md_BidVol_[10];
    int    delta_md_AskVol_[10];

public:
    double avgTradeBidVol_;
    double avgTradeAskVol_;
    
private:
    double CR(double csignal, double newsignal, double time_diff, double horizon);
    double BSqrt(double x);
    void   GetLogSkew(const CMarketStruct & ms);
    void   GetRealTradeVol(const CMarketStruct & ms);
    double Solvelinear(int lastvol, int turnover, int price_low, int price_high);
    void   GetBookVol(const CMarketStruct & ms);
    void   GetBookSignal();
    void   GetTradeSignal();
    void   GetCumReturn(const CMarketStruct & ms);
    void   GetOBDMap2(const CMarketStruct & ms);
    void   GetOBDSignal(const CMarketStruct & ms);    
    void   GetAvgTradeVol(const CMarketStruct & ms);
    
public:
    CBaseSignal();
    ~CBaseSignal();
    bool   init(string config_path);
    void   OnTick(const CMarketStruct & ms);
    double GetMut(double comp_logskew_coeff, double *trade_coeff, double *book_coeff, double *OBD_coeff) const;
    double RetCompositeAskVol() const;
    double RetCompositeBidVol() const;
    double RetTradeAskVol() const;
    double RetTradeBidVol() const;
    double RetLogSkew() const;
    double RetTrade(int period_index) const;
    double RetBook(int period_index) const;
    double RetOBD(int period_index) const;  
    double RetSkewalpha() const;
    int type_;
};
#endif