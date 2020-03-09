#include "CLossModel.h"
#include "math.h"
#include "CTradeBaseInfo.h"
#include "CTimer.h"
#include <thread>
#include <chrono>
#include <fstream>


CLossModel::CLossModel()
{
    levels_[3] = -2;
    levels_[2] = -1;
    levels_[1] = 0;
    levels_[0] = 1;
    for(int count=0; count< TOTAL_DELTA_; count++){
        loss_level_[count] = 0.0;
    }
    std::ifstream f1("CLossModelCoeff.cnf");
    if (!f1){
        	cout << "can't read file: CLossModelCoeff.cnf" << endl;
    }	
    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();
    json param_reader;
    param_reader = json::parse(content);
    valid_duration_ = param_reader["valid_duration"];
    volume_multiplier_ = param_reader["volume_multiplier"];
    maxordervolume_ = param_reader["maxordervolume"];
    leastordervolume_ = param_reader["leastordervolume"];
    extra_slippage_ = param_reader["extra_slippage"];
    instr_hash_ = 0;
    totalsize_ = 0;
    order_placetime_ = 0;
    timeleft_ = 0.0;
    duration_ = 0.0;
    priceup_ = 10000;
    pricedown_ = 1;
    isOrdering_ = false;
    direction_ = 1;
    bestordervol_ = 0;
    bestorderprice_ = 0;
    QorderID_size_ = 0;
    uv_ = 0;
    //lastorderprice_ = -1;
    cancel_status_ = CANCEL_STATUS_NORMAL;
    for(int count =0; count < MAX_OUTSTANDING_ORDERS; ++count){
        QorderID_[count] = -1;
    }
}

CLossModel::~CLossModel(){}

int CLossModel::GetSizeplacedOnPrice(int direction, int orderprice){ // get the waiting size ahead
    int placedsize = 0;
    placedsize = ms_.GetTradePriceRelatedVolume(orderprice, direction);
    if(placedsize < 0){
        placedsize = 0;
    }
    if(orderpvmap_.find(orderprice) != orderpvmap_.end()){        
        placedsize = placedsize + orderpvmap_[orderprice];
    }
    return placedsize;
}

double CLossModel::Lossfun(int level_index, int direction, double timeleft, int orderprice){ //
    double adapted_mut = LinearInterpolate(dm_.RetStartHorizon(), dm_.RetStartMut(), dm_.RetEndHorizon(), dm_.RetEndMut(), timeleft);
    double sigma_t = vm_.RetInstantVolatility();
    double spread = avgsp_.GetAvgspread();
    int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
    double delta = (orderprice - midpriceT2/2.0)*(-direction);
    double factor1 = -delta;
    
    double factor0 = extra_slippage_ + direction * abm_.ExpectedSt(adapted_mut, sigma_t, delta, spread/2.0, direction);
    int placedsize = GetSizeplacedOnPrice(direction, orderprice);
    double factor2 = lp_.EvaluateProb(level_index, placedsize, bs_, sigma_t, timeleft, valid_duration_, direction); // prob multi-size
    double ExeProb = direction >0 ? abm_.GetBuyCABMExecutedProb(adapted_mut, sigma_t, delta + spread/2.0) 
                                  : abm_.GetSellCABMExecutedProb(adapted_mut, sigma_t, delta + spread/2.0);
    
    if(ExeProb < 1.0){
        factor2 = (factor2 + ExeProb)/2.0;
    }    
    return factor1 * factor2 + factor0 *(1.0 - factor2);
}

int CLossModel::GetLossMinimizer(int direction, double timeleft, int newplacedsize){ //argmin optimizer over price
    for(int pointer = 0; pointer < TOTAL_DELTA_; pointer++){
        loss_level_[pointer] = 0;
    }
    for(int level_index = 0; level_index < TOTAL_DELTA_; level_index++){
        orderpvmap_.clear();
        int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
        int orderprice = GetExecutablePrice(midpriceT2, levels_[level_index], direction);
        orderpvmap_.insert(pair<int,int>(orderprice, newplacedsize));
        loss_level_[level_index] =  Lossfun(level_index, direction, timeleft, orderprice);
    }
    
    double tmp = loss_level_[0];
    int min_index = 0;
    for(int level_index = 1; level_index < TOTAL_DELTA_; level_index++){
        if(tmp > loss_level_[level_index]){
            tmp = loss_level_[level_index];
            min_index = level_index;
        }
    }    
    return min_index;
}

void CLossModel::GetBestOrderPriceVolume(){ // 
    if (timeleft_ > DURATION_LIMIT){      
        int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
        int bestlevelIndex = GetLossMinimizer(direction_, timeleft_, volume_multiplier_);
        int bestdelta = levels_[bestlevelIndex];
        double bestloss = loss_level_[bestlevelIndex];
        
        if(bestdelta == -2 and ((loss_level_[0] - loss_level_[bestlevelIndex]) < LOSSVALUE_DIFF_THRESHOLD)
                           and (ms_.askp_[0] - ms_.bidp_[0] == 1) and (ms_.askv_[0] < ms_.bidv_[0]) ){
            bestdelta = 1;
            bestlevelIndex = 0;
        }
        
//        int level_idx =0;
//        for(; level_idx < TOTAL_DELTA_; level_idx++){
//            if(lastbestorderprice == GetExecutablePrice(midpriceT2, levels_[level_idx], direction_)){
//                break;
//            }
//        }      
//        lastbestlossvalue = loss_level_[level_idx];       // one can think of changing this into unordered map 
//        if(abs(lastbestlossvalue - bestloss) > LOSSVALUE_CHANGE_THRESHOLD){
//            orderp_switch_status = true;
//        }
//        else{
//            orderp_switch_status = false;
//        }
        
        int bestorderprice = GetExecutablePrice(midpriceT2, bestdelta, direction_);
        if(direction_ > 0 and (ms_.bidp_[0] == priceup_) and (bestlevelIndex ==0) and (ms_.askp_[0] - ms_.bidp_[0] == 1) ){
            bestlevelIndex = -1;            
        }else if(direction_ < 0 and (ms_.askp_[0] == pricedown_) and (bestlevelIndex == 0) and (ms_.askp_[0] - ms_.bidp_[0] == 1) ){
            bestlevelIndex = -1;
        }
        
        int ordervolume = lq_.GetOrdervolume(leastordervolume_, maxordervolume_, volume_multiplier_, bestlevelIndex, direction_, dm_, vm_);
        ordervolume = bestorderprice > priceup_ ? 0 : (bestorderprice < pricedown_ ? 0 : ordervolume);
        bestorderprice = bestorderprice > priceup_ ? priceup_ : (bestorderprice < pricedown_ ? pricedown_ : bestorderprice);
        
        ordervolume = ordervolume > totalsize_ ? totalsize_ : ordervolume;
        
        bestorderprice_ = bestorderprice;
        bestordervol_ = ordervolume;
        
//        if(orderp_switch_status){
//            bestorderprice_ = bestorderprice;
//        }
//        else{
//            bestorderprice_ = lastbestorderprice;
//        }
//        double ratio = direction_ > 0 ? ms_.askv_[0]/max(1.0, bs_.avgTradeAskVol_) : ms_.bidv_[0]/max(1.0, bs_.avgTradeBidVol_);
//        string log = parseNano(ms_.exchtime_ * 1000000000, "%H:%M:%S") + "," + std::to_string(long(ms_.exchtime_ * 1000)%1000) + "," + std::to_string(ratio)
//        + "," + std::to_string(timeleft_) + "," + std::to_string(bestdelta)
//        + "," + std::to_string(lastbestorderprice) + "," + std::to_string(orderp_switch_status)
//        + "," + std::to_string(lastbestlossvalue)  + "," + std::to_string(bestloss)
//        + "," + std::to_string(totalsize_) + "," + std::to_string(bestordervol_) + "," + std::to_string(bestorderprice_);

        string log = parseNano(ms_.exchtime_ * 1000000000, "%H:%M:%S") + "," + std::to_string(long(ms_.exchtime_ * 1000)%1000)
        + "," + std::to_string(timeleft_) + "," + std::to_string(bestdelta)
        + "," + std::to_string(bestloss)+ "," + std::to_string(loss_level_[0]) + "," + std::to_string(loss_level_[1])
        + "," + std::to_string(loss_level_[2]) +"," + std::to_string(loss_level_[3]) 
        + "," + std::to_string(totalsize_) + "," + std::to_string(bestordervol_) + "," + std::to_string(bestorderprice_) + ",ordersize " +  std::to_string(QorderID_size_);
        std::cout << log  << endl;        
        //fastLog(log_id, log);        
        //lastbestorderprice = bestorderprice_;        
    }
    else{    
        bestorderprice_ = direction_ >0 ? ms_.askp_[0] : ms_.bidp_[0];
        bestordervol_ = totalsize_;
    }
}

void CLossModel::PlaceOrder(){    // after bestorderprice,bestordervolume, start to order
    emOffset em_status;
    em_status = emOffset::AUTO;
    const tInstrumentInfo instr_info = CTradeBaseInfo::instr_info_[instr_hash_];    
    double ticksize = instr_info.tick_price;
    
    if(QorderID_size_ == 0){ // if there is no pre-ordering, then order
        if(bestordervol_ > 0){
            char direction_char = direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
            long orderid = sendOrder(instr_hash_, bestorderprice_ * ticksize, bestordervol_, direction_char, (int)em_status);
            std::cout << "place order " << bestorderprice_ * ticksize << " ,vol:" << bestordervol_ << ", size left: " << totalsize_ << endl;
            QorderID_[0] = orderid;
            ++QorderID_size_;
            //lastorderprice_ = bestorderprice_;            
        }
    }
    else{
        RiskStg* p_risk = (RiskStg*) getAccountObj();
        uv_ = p_risk->getInstrUnitVol(instr_hash_);
        int placed_volume = (direction_ >0 ? (uv_->pos_long_td_opn_ing + uv_->pos_long_td_cls_ing + uv_->pos_long_yd_cls_ing): (uv_->pos_short_td_opn_ing + uv_->pos_short_td_cls_ing + uv_->pos_short_yd_cls_ing));
        int APIStatus = CancelOrderAPIStatus(bestorderprice_ * ticksize);
        if(APIStatus == 1){// if successfully sent to exchange to cancel
            std::cout << "try to send reorder " << bestorderprice_ * ticksize << " ,vol:" << bestordervol_  << " placed vol:" <<  placed_volume << endl;
            if((bestordervol_ > placed_volume) and (QorderID_size_ <= MAX_OUTSTANDING_ORDERS)){
                char direction_char = direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
                long orderid = sendOrder(instr_hash_, bestorderprice_ * ticksize, bestordervol_ - placed_volume, direction_char, (int)em_status);                
                std::cout << "send reorder " << bestorderprice_ * ticksize << " ,vol:" << bestordervol_ - placed_volume << ", size left: " << totalsize_ << endl;
                int non_occupied_count = 0;            
                while(QorderID_[non_occupied_count] >0)
                    ++non_occupied_count;                
                QorderID_[non_occupied_count] = orderid;
                
                ++QorderID_size_;
                //lastorderprice_ = bestorderprice_;
            }
        }    
    }
}

int CLossModel::CancelOrderAPIStatus(double to_orderprice){ // check if it needs to cancel
    int api_status = 1;
    int count = 0;
    //if(lastorderprice_ != bestorderprice_){
    while(count < MAX_OUTSTANDING_ORDERS){        
        if(QorderID_[count] >= 0){
            tOrderTrack& ordertracker = getOrderTrack(QorderID_[count]);
            if(ordertracker.price != to_orderprice){
                api_status = cancelOrder(QorderID_[count]);
                std::cout << "try to cancel: " << to_orderprice << "," << ordertracker.price  << endl;
            }            
        }
        ++count;
    }
    //}
    return api_status;
}

void CLossModel::sys_on_rtn(const tRtnMsg* rtn){ // response on return
    emOffset em_status;
    em_status = emOffset::AUTO;
    const tInstrumentInfo instr_info = CTradeBaseInfo::instr_info_[rtn->instr_hash];
    double ticksize = instr_info.tick_price;
    
    if(rtn->msg_type == (int)(emOrderRtnType::EXECUTION)){
        totalsize_ -= rtn->vol;
        int count = 0;
        while((QorderID_[count] != rtn->order_ref)){
            ++count;
        } 
        tOrderTrack& ordertracker = getOrderTrack(rtn->order_ref);
        if (ordertracker.vol == ordertracker.vol_traded){            
            QorderID_[count] = -1;
            --QorderID_size_;
        }
        //int executep = rtn->price/ticksize;
        //std::cout << "execution, execute price," << executep << ", traded:" << ordertracker.vol_traded << endl;
    }
    
    if(rtn->msg_type == (int)emOrderRtnType::CANCEL_REJECT){
        std::cout << " cancel reject" << endl;
    }
    
    if(rtn->msg_type == (int)emOrderRtnType::CANCELED and cancel_status_ == CANCEL_STATUS_NORMAL){
        std::cout << " CANCELED:" << rtn->order_ref << endl;     
        int count = 0;
        while(QorderID_[count] != rtn->order_ref){
            ++count;
        }
        QorderID_[count] = -1;
        --QorderID_size_;        
        if(QorderID_size_ == 0){ // if all orders are cancelled, then reorder, orders after cancel orders;
            //lastorderprice_ = -1;
            if(bestordervol_ >0){
                char direction_char = direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
                long orderid = sendOrder(rtn->instr_hash, bestorderprice_ * ticksize, (bestordervol_ < totalsize_ ? bestordervol_ : totalsize_), direction_char, (int)em_status);
                std::cout << "cancel reorder " << bestorderprice_ * ticksize << " ,vol:" << (bestordervol_ < totalsize_ ? bestordervol_ : totalsize_) <<  ", size left: " << totalsize_  << endl;
                QorderID_[0] = orderid;
                ++QorderID_size_;
                //lastorderprice_ = bestorderprice_;
            }        
        }        
    }
}

void CLossModel::ProcessExit(){    // process if time is over
    if(timeleft_ < -0.1){
        if(totalsize_ > 0){
            std::cout << "ProcessExit, and time left: " << timeleft_ << endl;
        }
           
        for(int count=0; count < MAX_OUTSTANDING_ORDERS; count++){
            if(QorderID_[count] != -1){
                int ap_status = cancelOrder(QorderID_[count]);
                cancel_status_ = CANCEL_STATUS_EXIT;
                QorderID_[count] = -1;
            }
        }
        QorderID_size_ = 0;
        totalsize_ = 0;
        isOrdering_ = false;
    }
}

void CLossModel::OnTick(const UnitedMarketData *md){ // 
    instr_hash_ = md->instr_hash;
    
    ms_.OnTick(md);
    if(ms_.RetIsLastReady_()){        
        bs_.OnTick(ms_);    
        ss_.OnTick(ms_);
        dm_.OnTick(timeleft_, bs_);
        vm_.OnTick(timeleft_, valid_duration_, ss_);
        avgsp_.OnTick(ms_);                         
        if(isOrdering_){
            GetBestOrderPriceVolume();            
            PlaceOrder();        
        }
    }
    ms_.UpdateLastTickInfo(md);
}

void CLossModel::GetTimeleft(){
    if(isOrdering_){
        timeleft_ = duration_ - (CTimer::instance().getNano()/1000000000.0 - order_placetime_);
    }
    else{
        timeleft_ = duration_;
    }
    std::cout << "time left:" << timeleft_ << endl;
    if(timeleft_ >= valid_duration_){
        timeleft_ = valid_duration_;
    }
}

void CLossModel::sys_on_tick(const UnitedMarketData* md){
    GetTimeleft();
    OnTick(md);
}

void CLossModel::sys_on_time(long nano){
    ProcessExit();    
}

void CLossModel::sys_on_switch_day(string day){}

void CLossModel::OnExecution(int priceup, int pricedown, int placedsize, int direction, double duration){ // for higher level strategy usage 
    isOrdering_ = true; 
    priceup_ = priceup;
    pricedown_ = pricedown;
    totalsize_ = placedsize;
    direction_ = direction;
    duration_  = duration;
    cancel_status_ = CANCEL_STATUS_NORMAL;
    order_placetime_ = CTimer::instance().getNano()/1000000000.0;  
    GetBestOrderPriceVolume();            
    PlaceOrder();   
}
