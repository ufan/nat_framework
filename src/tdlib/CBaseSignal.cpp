#include "CBaseSignal.h"
#include "math.h"
#include <thread>
#include <chrono> 
#include <string> 
#include<CTimer.h>

//extern int log_id;

CBaseSignal::CBaseSignal()
{
    for(int count =0; count < TOTAL_HORIZON_; count++){
            horizons_[count] = 100;
    }    
    horizons_[0] = 5;
    horizons_[1] = 10;
    horizons_[2] = 20;
    horizons_[3] = 60;    
    OBD_alpha_ = 0.1;
    
    for(int count =0; count < TOTAL_HORIZON_; count++){
        OBD_[count] = 0;
        trade_[count] = 0;
        book_[count] = 0;
        CR_[count] = 0;
    }
    
    for(int count = 0; count< 10; count++){
        delta_md_Bid_[count] = 0;
        delta_md_Ask_[count] = 0;
        delta_md_BidVol_[count] = 0;
        delta_md_AskVol_[count] = 0;
    }
    timediff_ = 0;
    
    avgTradeAskVol_ = 0;
    avgTradeBidVol_= 0;
        
    std::cout << "CBaseSignal::Init" <<endl;
}

CBaseSignal::~CBaseSignal(){}

void CBaseSignal::GetLogSkew(const CMarketStruct & ms){ // apply to any 
    composite_bidvol_ = ms.bidv_[0];
    composite_askvol_ = ms.askv_[0];
    
    if(LEVELCOUNT_ > 1){
        for(int level=1; level < LEVELCOUNT_; level++){
            composite_bidvol_ += ms.bidv_[level] * pow(2.0, -skewalpha_ *(ms.bidp_[0] - ms.bidp_[level]));        
        }
        for(int level=1; level < LEVELCOUNT_; level++){
            composite_askvol_ += ms.askv_[level] * pow(2.0, -skewalpha_ *(-ms.askp_[0] + ms.askp_[level]));        
        }
    }    
    composite_logskew_ = (composite_bidvol_ - composite_askvol_)/(composite_bidvol_ + composite_askvol_) * log(composite_bidvol_ + composite_askvol_);
}

void CBaseSignal::GetRealTradeVol(const CMarketStruct & ms){
    int Bt1 = ms.bidp_[0];
    int Bt = ms.lastbidp_[0];
    int At1 = ms.askp_[0];
    int At = ms.lastaskp_[0];
    int BVt = ms.lastbidv_[0];
    int AVt = ms.lastaskv_[0];
    double avg = ms.avgprice_;
    int tradevol = ms.tradevol_;
    int turnover = ms.turnover_;
    double eps = 0.0001;
    type_ = -1;   
    
    if(avg < Bt and avg < Bt1){
        double w1 = (avg - Bt)/(avg- Bt + avg - Bt1);
        realtradebidvol_ = w1 * tradevol;
        type_ = 1;
    }
    if(avg > At and avg > At1){
        double w1 = (avg - At)/(avg- At + avg - At1);
        realtradebidvol_ = (1- w1)* tradevol;
        type_ = 2;
    }
    if(Bt <= Bt1 and Bt1 < avg and avg <= At1 and At1 <= At){
        realtradebidvol_ = Solvelinear(tradevol, turnover, Bt1, At1);
        type_ = 3;
    }
            
    if((Bt <= avg) and (avg <= Bt1) and (At1 <= At)){
        realtradebidvol_ = Solvelinear(tradevol, turnover, Bt, At1);
        type_ = 4;
    }
            
    if((Bt <= Bt1) and (At1 <= avg) and (avg <= At)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt1, At);
        type_ = 5;
    }
    
    if((Bt <= Bt1) and (Bt1 <= avg) and (avg < At) and (At <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt1, At);
        type_ = 6;
    }
    
    if((Bt <= avg) and (avg <= Bt1) and (Bt1 <= At) and (At <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt, At);
        type_ = 7;
    }
    
    if((Bt <= Bt1) and (Bt1 <= At) and (At <= avg) and (avg <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt1, At1);        
        type_ = 8;
    }
    
    if(At <= Bt1){
        realtradebidvol_ = 0;
        type_ = 9;
    }
    
    if((Bt1 <= Bt) and (Bt < avg) and (avg <= At) and (At <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt, At);
        type_ = 10;
    }
    
    if((Bt1 <= avg) and (avg <= Bt) and (At <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt1, At);    
        type_ = 11;
    }
    
    if((Bt1 <= Bt) and (At <= avg) and (avg <= At1)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt, At1);
        type_ = 12;
    }
    
    if((Bt1 <= Bt) and (Bt <= avg) and (avg < At1) and (At1 <= At)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt, At1);
        type_ = 13;
    }
    
    if((Bt1 <= avg) and (avg <= Bt) and (Bt <= At1) and (At1 <= At)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt1, At1);
        type_ = 14;
    }
        
    if((Bt1 <= Bt) and (Bt <= At1) and (At1 <= avg) and (avg <= At)){
        realtradebidvol_ = Solvelinear(tradevol, turnover,Bt, At);
        type_ = 15;
    }
    
    if(At1 <= Bt){
        realtradebidvol_ = tradevol;
        type_ = 16;
    }

    if(tradevol == 0){
        realtradebidvol_ = 0;
        type_ = 0;
    }
        
    if((abs(avg - At) < eps) and (tradevol >= AVt)){
        realtradebidvol_ = 0;
        type_ = 17;
    }
    
    if((abs(avg - Bt) < eps) and (tradevol >= BVt)){
        realtradebidvol_ = tradevol;
        type_ = 18;
    }
        
    if((abs(avg - At) < eps) and (abs(avg - At1) < eps)){
        realtradebidvol_ = 0;
        type_ = 19;
    }

    if((abs(avg - Bt) < eps) and (abs(avg - Bt1) < eps)){
        realtradeaskvol_ = 0;
        type_ = 20;
    }
    realtradeaskvol_ = tradevol - realtradebidvol_;
}

double CBaseSignal::Solvelinear(int lastvol,int turnover,int price_low,int price_high){
    return (0.0 + turnover - price_high * lastvol)/(-price_high + price_low);    
}

double CBaseSignal::BSqrt(double x){
    return x<0 ? -sqrt(-x) : sqrt(x);
}

double CBaseSignal::CR(double csignal, double newsignal, double time_diff, double horizon){
    return newsignal + pow(2.0, -abs(time_diff)/horizon) * csignal;    
}

void CBaseSignal::GetBookVol(const CMarketStruct & ms){
    bookbid_ = 0;
    bookask_ = 0;
    if(ms.bidp_[0] == ms.lastbidp_[0]){
        bookbid_ = BSqrt(ms.bidv_[0] - ms.lastbidv_[0] + realtradebidvol_);
    }
    else if(ms.bidp_[0] > ms.lastbidp_[0]){
        bookbid_ = sqrt(ms.bidv_[0]);
    }
        
    if(ms.askp_[0] == ms.lastaskp_[0]){
        bookask_ = BSqrt(ms.askv_[0] - ms.lastaskv_[0] + realtradeaskvol_);
    }
    else if(ms.askp_[0] < ms.lastaskp_[0]){
        bookask_ = sqrt(ms.askv_[0]);
    }
}

void CBaseSignal::GetBookSignal(){
    //double newsignal = sqrt(bookbid_) - sqrt(bookask_);
    double newsignal = bookbid_ - bookask_;
    
    for (int count = 0; count < HORIZON_USE_; count++){
        book_[count] = CR(book_[count], newsignal, timediff_, horizons_[count]);
    }
}

void CBaseSignal::GetTradeSignal(){
    double newsignal = sqrt(realtradeaskvol_) - sqrt(realtradebidvol_);
    for (int count = 0; count < HORIZON_USE_; count++){
        trade_[count] = CR(trade_[count], newsignal, timediff_, horizons_[count]);
    }
}

void CBaseSignal::GetCumReturn(const CMarketStruct & ms){
    double newsignal = (ms.askp_[0] + ms.bidp_[0] - ms.lastaskp_[0] - ms.lastbidp_[0])/2.0; //Mid CumReturn
    for (int count = 0; count < HORIZON_USE_; count++){
        CR_[count] = CR(CR_[count], newsignal, timediff_, horizons_[count]);
    }
}

void CBaseSignal::GetOBDMap2(const CMarketStruct & ms){
    for(int count = 0; count< 10; count++){
        delta_md_Bid_[count] = 0;
        delta_md_Ask_[count] = 0;
        delta_md_BidVol_[count] = 0;
        delta_md_AskVol_[count] = 0;
    }
    int i = 0, j = 0, k = 0;
	while (i < 5 && ms.lastbidp_[i] > ms.bidp_[0])	 
		++i;
	while (j < 5 && ms.bidp_[j] > ms.lastbidp_[0])
		++j;
	
	while(i < 5 && j < 5){
		delta_md_Bid_[k] = ms.lastbidp_[i] > ms.bidp_[j] ? ms.lastbidp_[i] : ms.bidp_[j];		
		if (delta_md_Bid_[k] == ms.lastbidp_[i]){
        		delta_md_BidVol_[k] -= ms.lastbidv_[i];
			++i;
		}
		if(delta_md_Bid_[k] == ms.bidp_[j]){
        		delta_md_BidVol_[k] += ms.bidv_[j];
        		++j;
		}
		++k;
	}	
	i = j = k = 0;
	while (i < 5 && ms.lastaskp_[i] < ms.askp_[0])		 
		++i;
	while (j < 5 && ms.askp_[j] < ms.lastaskp_[0])
		++j;
		
	while (i < 5 && j < 5){
		delta_md_Ask_[k] = ms.lastaskp_[i] < ms.askp_[j] ? ms.lastaskp_[i] : ms.askp_[j];
		if (delta_md_Ask_[k] == ms.lastaskp_[i]){
			delta_md_AskVol_[k] -= ms.lastaskv_[i];
			++i;
		}
		if (delta_md_Ask_[k] == ms.askp_[j]){
			delta_md_AskVol_[k] += ms.askv_[j];
			++j;
		}
		++k;
	}
}

void CBaseSignal::GetOBDSignal(const CMarketStruct & ms){  
    double bidchg = 0;
    double askchg = 0;
    for(int pointer =0; pointer <10; pointer++){
        if(delta_md_Bid_[pointer] >0 && delta_md_BidVol_[pointer] >0){
            bidchg += exp(-OBD_alpha_ * abs(-delta_md_Bid_[pointer] + ms.bidp_[0])) * delta_md_BidVol_[pointer];
        }
    }
    
    for(int pointer = 0; pointer <10; pointer++){
        if(delta_md_Ask_[pointer] >0 && delta_md_AskVol_[pointer] >0){
            askchg += exp(-OBD_alpha_ * abs(delta_md_Ask_[pointer] - ms.askp_[0])) * delta_md_AskVol_[pointer];
        }
    }
    
    bidchg = BSqrt(bidchg);
    askchg = BSqrt(askchg);
    double newsignal = bidchg - askchg;
    for (int count=0; count < TOTAL_HORIZON_; count++)
        OBD_[count] = CR(OBD_[count], newsignal, timediff_, horizons_[count]);
}

void CBaseSignal::GetAvgTradeVol(const CMarketStruct & ms){
    avgTradeAskVol_ =  EWM(avgTradeAskVol_, realtradeaskvol_, TRADEVOL_HORIZON, ms.RetWhichTick_()); // subjectively buy
    avgTradeBidVol_ =  EWM(avgTradeBidVol_, realtradebidvol_, TRADEVOL_HORIZON, ms.RetWhichTick_()); // subjectively sell
}

void CBaseSignal::OnTick(const CMarketStruct & ms){
    timediff_ = ms.exchtime_ - ms.lastexchtime_;
    GetLogSkew(ms);
    GetCumReturn(ms);
    GetRealTradeVol(ms);
    GetTradeSignal();
    GetBookVol(ms);
    GetBookSignal();
    GetAvgTradeVol(ms);
    if(LEVELCOUNT_ == 5){
        GetOBDMap2(ms);
        GetOBDSignal(ms);
    }          
}

double CBaseSignal::GetMut(double composite_logskew_coeff, double *trade_coeff, double *book_coeff, double *OBD_coeff) const{
    double mu_t= 0;
    mu_t += composite_logskew_coeff * composite_logskew_;
    
    for(int count=0; count < HORIZON_USE_; count++){
        mu_t += trade_coeff[count] * trade_[count];
        mu_t += book_coeff[count] * book_[count];
        if(LEVELCOUNT_ == 5){
            mu_t += OBD_coeff[count] * OBD_[count];        
        }
    }    
    return mu_t;
}

double CBaseSignal::RetCompositeAskVol() const{
    return composite_askvol_;    
}

double CBaseSignal::RetCompositeBidVol() const{
    return composite_bidvol_;
}
    
double CBaseSignal::RetTradeAskVol() const{
    return realtradeaskvol_;
}

double CBaseSignal::RetTradeBidVol() const{
    return realtradebidvol_;
}

double CBaseSignal::RetLogSkew() const{
    return composite_logskew_;
}

double CBaseSignal::RetTrade(int period_index) const{
    return trade_[period_index];
}

double CBaseSignal::RetBook(int period_index) const{
    return book_[period_index];
}

double CBaseSignal::RetOBD(int period_index) const{
    return OBD_[period_index];
}

double CBaseSignal::RetSkewalpha() const{
    return skewalpha_;
}

/*void CBaseSignal::GetOBDMap(const CMarketStruct & ms){
    // lastpv map is initialized, this is for multi-levels only
    map<int,int> askpvmap;
    map<int,int> bidpvmap;
    map<int,int> lastaskpvmap;
    map<int,int> lastbidpvmap;     
    askchangemap_.clear();
    bidchangemap_.clear();
    
    for(int count =0; count< 5; count++){
        askpvmap.insert(std::pair<int,int>(ms.askp_[count], ms.askv_[count]));
        bidpvmap.insert(std::pair<int,int>(ms.bidp_[count], ms.bidv_[count]));
        lastbidpvmap.insert(std::pair<int,int>(ms.lastbidp_[count], ms.lastbidv_[count]));
        lastaskpvmap.insert(std::pair<int,int>(ms.lastaskp_[count], ms.lastaskv_[count]));
    }
      
    for(int count =0; count<5; count++){
        int px = ms.lastbidp_[count];
        if ((bidpvmap.find(px) == bidpvmap.end()) && (px <= ms.bidp_[0]) && (px >= ms.bidp_[4]) && (px >= ms.lastbidp_[4]))
            bidpvmap.insert(std::pair<int,int>(px, 0));
        if(ms.bidp_[count] < ms.bidp_[4])
            bidpvmap[ms.bidp_[count]] = 0;
    }
    for(int count =0; count<5; count++) {
        int px = ms.lastaskp_[count];
        if ((askpvmap.find(px) == askpvmap.end()) && (px >= ms.askp_[0]) && (px <= ms.askp_[4]) && (px <= ms.lastaskp_[4]))
            askpvmap.insert(std::pair<int,int>(px, 0));
                                   
        if(ms.askp_[count] > ms.askp_[4])
            askpvmap[ms.askp_[count]] = 0;
    }
    
    for(map<int,int>::iterator it : askpvmap){
        if(lastaskpvmap.find(it->first) == lastaskpvmap.end()){            
            askchangemap_.insert(std::pair<int,int>(it->first, askpvmap[it->first]));
        }else{
            askchangemap_.insert(std::pair<int,int>(it->first, askpvmap[it->first] - lastaskpvmap[it->first]));
        }
    }    
    for(map<int,int>::iterator it : bidpvmap){
        if(lastbidpvmap.find(it->first) == lastbidpvmap.end()){
            bidchangemap_.insert(std::pair<int,int>(it->first, bidpvmap[it->first]));  
        }else{
            bidchangemap_.insert(std::pair<int,int>(it->first, bidpvmap[it->first] - lastbidpvmap[it->first]));
        }
    }    
} */