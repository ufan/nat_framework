#include "CLossModel.h"
#include "math.h"
#include "CTradeBaseInfo.h"
#include "CTimer.h"
#include <thread>
#include <chrono>
#include <fstream>


CLossModel::CLossModel(CStrategy *powner) : p_owner_(powner)
{
    levels_[3] = -2;
    levels_[2] = -1;
    levels_[1] = 0;
    levels_[0] = 1;

    orders_.reserve(500);
}

CLossModel::~CLossModel(){}

bool CLossModel::init(string instr, string config)
{
    instr_hash_ = INSTR_STR_TO_HASH(instr);
	if(CTradeBaseInfo::instr_info_.find(instr_hash_) == CTradeBaseInfo::instr_info_.end())
	{
		LOG_ERR("cannot find instr: %s", instr.c_str());
		return false;
	}
    const tInstrumentInfo instr_info = CTradeBaseInfo::instr_info_[instr_hash_];
    ticksize_ = instr_info.tick_price;

	json param_reader = json::parse(config);
	ASSERT_RET(loadLossModelConfig(param_reader["/ExecModel/loss_model_conf_path"_json_pointer]), false);
	ASSERT_RET(dm_.init(param_reader["/ExecModel/drift_model_conf_path"_json_pointer]), false);
	ASSERT_RET(lq_.init(param_reader["/ExecModel/liquidity_model_conf_path"_json_pointer]), false);
	ASSERT_RET(lp_.init(param_reader["/ExecModel/logitp_model_conf_path"_json_pointer]), false);
	ASSERT_RET(ms_.init(instr.c_str()), false);
	ASSERT_RET(vm_.init(param_reader["/ExecModel/volatility_model_conf_path"_json_pointer]), false);
	ASSERT_RET(bs_.init(param_reader["/ExecModel/basesignal_model_conf_path"_json_pointer]), false);
	
	return true;
}

bool CLossModel::loadLossModelConfig(string config)
{
	std::ifstream f1(config);
	if (!f1)
	{
		LOG_ERR("can't read file: %s", config.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
	f1.close();
	json param_reader = json::parse(content);
	valid_duration_ = param_reader["valid_duration"];
	volume_multiplier_ = param_reader["volume_multiplier"];
	maxordervolume_ = param_reader["maxordervolume"];
	leastordervolume_ = param_reader["leastordervolume"];
	extra_slippage_ = param_reader["extra_slippage"];
	
	for(int count=0; count< TOTAL_DELTA_; count++){
        levels_[count] = param_reader["PRICELEVEL"][count];
    }
    
	return true;
}

int CLossModel::GetSizeplacedOnPrice(tExecOrder &eo, int orderprice){ // get the waiting size ahead
    int placedsize = 0;
    placedsize = ms_.GetTradePriceRelatedVolume(orderprice, eo.direction_);
    if(placedsize < 0){
        placedsize = 0;
    }
    if(eo.orderpvmap_.find(orderprice) != eo.orderpvmap_.end()){
        placedsize = placedsize + eo.orderpvmap_[orderprice];
    }
    return placedsize;
}

double CLossModel::Lossfun(tExecOrder &eo, int level_index, int orderprice){ //
    double adapted_mut = LinearInterpolate(dm_.RetStartHorizon(), dm_.RetStartMut(), dm_.RetEndHorizon(), dm_.RetEndMut(), eo.timeleft_);
    double sigma_t = vm_.RetInstantVolatility(eo.timeleft_, valid_duration_);
    double spread = avgsp_.GetAvgspread();
    int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
    double delta = (orderprice - midpriceT2/2.0)*(-eo.direction_);
    double factor1 = -delta;
    
    double factor0 = extra_slippage_ + eo.direction_ * abm_.ExpectedSt(adapted_mut, sigma_t, delta, spread/2.0, eo.direction_);
    int placedsize = GetSizeplacedOnPrice(eo, orderprice);
    double factor2 = lp_.EvaluateProb(level_index, placedsize, bs_, eo.timeleft_, valid_duration_, eo.direction_); // prob multi-size
    double ExeProb = eo.direction_ >0 ? abm_.GetBuyCABMExecutedProb(adapted_mut, sigma_t, delta + spread/2.0)
                                  : abm_.GetSellCABMExecutedProb(adapted_mut, sigma_t, delta + spread/2.0);
        
    if(ExeProb < 1.0){
        factor2 = (factor2 + ExeProb)/2.0;
    }
    
    LOG_LVL(DEBUGLEVEL3, "current_mu: %f, current sigma: %f, Estimated_St: %f, neutral_Estimated_Prob: %f, ABM_Estimated_Prob: %f", adapted_mut, sigma_t, factor0, factor2, ExeProb);
    LOG_LVL(DEBUGLEVEL3, "Estimated Executed Loss: %f, Estimated Non-Executed Loss: %f", factor1 * factor2,  factor0 *(1.0 - factor2));
    
    return factor1 * factor2 + factor0 *(1.0 - factor2);
}

int CLossModel::GetLossMinimizer(tExecOrder &eo, int newplacedsize){ //argmin optimizer over price
    for(int pointer = 0; pointer < TOTAL_DELTA_; pointer++){
        	eo.loss_level_[pointer] = 0;
    }
    for(int level_index = 0; level_index < TOTAL_DELTA_; level_index++){
    	eo.orderpvmap_.clear();
        int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
        int orderprice = GetExecutablePrice(midpriceT2, levels_[level_index], eo.direction_);
        eo.orderpvmap_.insert(pair<int,int>(orderprice, newplacedsize));
        eo.loss_level_[level_index] =  Lossfun(eo, level_index, orderprice);
    }
    
    double tmp = eo.loss_level_[0];
    int min_index = 0;
    for(int level_index = 1; level_index < TOTAL_DELTA_; level_index++){
        if(tmp > eo.loss_level_[level_index]){
            tmp = eo.loss_level_[level_index];
            min_index = level_index;
        }
    }    
    
    for(int level_index = 0; level_index < TOTAL_DELTA_; level_index++){
        LOG_LVL(DEBUGLEVEL3, "Estimated Loss for %d : %f", levels_[level_index],  eo.loss_level_[level_index]);    
    }
    
    return min_index;
}

void CLossModel::GetBestOrderPriceVolume(tExecOrder &eo){ //
    if (eo.timeleft_ > DURATION_LIMIT){
        int midpriceT2 = ms_.askp_[0] + ms_.bidp_[0];
        int bestlevelIndex = GetLossMinimizer(eo, volume_multiplier_);
        int bestdelta = levels_[bestlevelIndex];
        double bestloss = eo.loss_level_[bestlevelIndex];
        
        if(bestdelta == -2 and ((eo.loss_level_[0] - eo.loss_level_[bestlevelIndex]) < LOSSVALUE_DIFF_THRESHOLD)
                           and (ms_.askp_[0] - ms_.bidp_[0] == 1) and (ms_.askv_[0] < ms_.bidv_[0]) ){
            bestdelta = 1;
            bestlevelIndex = 0;
        }
        
        int bestorderprice = GetExecutablePrice(midpriceT2, bestdelta, eo.direction_);
        if(eo.direction_ > 0 and (ms_.bidp_[0] == eo.priceup_) and (bestlevelIndex ==0) and (ms_.askp_[0] - ms_.bidp_[0] == 1) ){
            bestlevelIndex = -1;            
        }else if(eo.direction_ < 0 and (ms_.askp_[0] == eo.pricedown_) and (bestlevelIndex == 0) and (ms_.askp_[0] - ms_.bidp_[0] == 1) ){
            bestlevelIndex = -1;
        }
        
        int ordervolume = lq_.GetOrdervolume(leastordervolume_, maxordervolume_, volume_multiplier_, bestlevelIndex, eo.direction_, dm_, vm_);
        ordervolume = bestorderprice > eo.priceup_ ? 0 : (bestorderprice < eo.pricedown_ ? 0 : ordervolume);
        bestorderprice = bestorderprice > eo.priceup_ ? eo.priceup_ : (bestorderprice < eo.pricedown_ ? eo.pricedown_ : bestorderprice);
        
        ordervolume = ordervolume > eo.totalsize_ ? eo.totalsize_ : ordervolume;
        
        eo.bestorderprice_ = bestorderprice;
        eo.bestordervol_ = ordervolume;

    }
    else{    
        	eo.bestorderprice_ = eo.direction_ >0 ? ms_.askp_[0] : ms_.bidp_[0];
        	eo.bestordervol_ = eo.totalsize_;
    }
}

void CLossModel::PlaceOrder(tExecOrder &eo){    // after bestorderprice,bestordervolume, start to order
    emOffset em_status;
    em_status = emOffset::AUTO;
    
    if(eo.QorderID_size_ == 0){ // if there is no pre-ordering, then order
        if(eo.bestordervol_ > 0){
            char direction_char = eo.direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
            long orderid = sendOrder(eo, eo.bestorderprice_ * ticksize_, eo.bestordervol_, direction_char, (int)em_status);
            eo.QorderID_[0] = orderid;
            ++eo.QorderID_size_;
        }
    }
    else{
        int APIStatus = CancelOrderAPIStatus(eo, eo.bestorderprice_ * ticksize_);
        if(APIStatus == 1){// if successfully sent to exchange to cancel
            if((eo.bestordervol_ > eo.placed_vol_) and (eo.QorderID_size_ < MAX_OUTSTANDING_ORDERS)){
                char direction_char = eo.direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
                long orderid = sendOrder(eo, eo.bestorderprice_ * ticksize_, eo.bestordervol_ - eo.placed_vol_, direction_char, (int)em_status);
                int non_occupied_count = 0;            
                while(eo.QorderID_[non_occupied_count] >0)
                    ++non_occupied_count;                
                eo.QorderID_[non_occupied_count] = orderid;
                
                ++eo.QorderID_size_;
            }
        }    
    }
}

int CLossModel::CancelOrderAPIStatus(tExecOrder &eo, double to_orderprice){ // check if it needs to cancel
    int api_status = 1;
    int count = 0;

    while(count < MAX_OUTSTANDING_ORDERS){        
        if(eo.QorderID_[count] >= 0){
            tOrderTrack& ordertracker = p_owner_->getOrderTrack(eo.QorderID_[count]);
            if(ordertracker.price != to_orderprice){
                api_status = p_owner_->cancelOrder(eo.QorderID_[count]);
            }            
        }
        ++count;
    }
    return api_status;
}

bool CLossModel::sys_on_rtn(const tRtnMsg* rtn){ // response on return
	for(auto &eo : orders_)
	{
		int count;
		for(count = 0; count < MAX_OUTSTANDING_ORDERS; count++)
		{
			if(eo.QorderID_[count] == rtn->order_ref) break;
		}
		if(count >= MAX_OUTSTANDING_ORDERS) continue;

		switch(rtn->msg_type)
		{
		case (int)emOrderRtnType::EXECUTION:
		{
			eo.placed_vol_ -= rtn->vol;
			eo.totalsize_ -= rtn->vol;
			tOrderTrack& ordertracker = p_owner_->getOrderTrack(rtn->order_ref);
			if (ordertracker.vol == ordertracker.vol_traded){
				eo.QorderID_[count] = -1;
				--eo.QorderID_size_;
			}
			LOG_LVL(DEBUGLEVEL3,"execution, execute price: %lf, traded: %d ",  rtn->price, rtn->vol);
			break;
		}
		case (int)emOrderRtnType::CANCELED:
		{
			eo.placed_vol_ -= rtn->vol;
			if(eo.cancel_status_ == CANCEL_STATUS_NORMAL)
			{
				LOG_LVL(DEBUGLEVEL3, " CANCELED: %d",  rtn->order_ref);
				eo.QorderID_[count] = -1;
				--eo.QorderID_size_;
				if(eo.QorderID_size_ == 0){ // if all orders are cancelled, then reorder, orders after cancel orders;
					if(eo.bestordervol_ > 0 && eo.totalsize_ > 0){
						char direction_char = eo.direction_ > 0 ? AT_CHAR_Buy : AT_CHAR_Sell;
						long orderid = sendOrder(eo, eo.bestorderprice_ * ticksize_, (eo.bestordervol_ < eo.totalsize_ ? eo.bestordervol_ : eo.totalsize_), direction_char, (int)emOffset::AUTO);
						LOG_LVL(DEBUGLEVEL3, "cancel reorder: %f ,vol: %d, size left: %d ", eo.bestorderprice_ * ticksize_ , (eo.bestordervol_ < eo.totalsize_ ? eo.bestordervol_ : eo.totalsize_), eo.totalsize_);
						eo.QorderID_[0] = orderid;
						++eo.QorderID_size_;
					}
				}
			}


			break;
		}
		default: ;
		}
		return true;
	}
    return false;
}

inline void CLossModel::exitExecution(int execid)
{
	auto &eo = orders_[execid];
	if(eo.isOrdering_)
	{
		if(eo.timeleft_ < -0.1 && eo.totalsize_ > 0){
			for(int count = 0; count < MAX_OUTSTANDING_ORDERS; count++){
				if(eo.QorderID_[count] != -1){
					int ap_status = p_owner_->cancelOrder(eo.QorderID_[count]);
					eo.cancel_status_ = CANCEL_STATUS_EXIT;
					eo.QorderID_[count] = -1;
				}
			}
			eo.QorderID_size_ = 0;
			eo.totalsize_ = 0;
			eo.isOrdering_ = false;
			LOG_LVL(DEBUGLEVEL1, "ExecutionOrder %d exit.", eo.order_id_);
		}
	}
}

void CLossModel::cancelExecution(int execid)
{
	if(execid >= 0 && execid < orders_.size())
	{
		exitExecution(execid);
	}
}

void CLossModel::ProcessExit(){    // process if time is over
	int len = orders_.size();
	for(int i = 0; i < len; i++)
	{
		exitExecution(i);
	}
}

void CLossModel::OnTick(const UnitedMarketData *md){ // 
    ms_.OnTick(md);
    if(ms_.RetIsLastReady_()){        
        bs_.OnTick(ms_);    
        ss_.OnTick(ms_);
        dm_.OnTick(bs_);
        vm_.OnTick(ss_);
        avgsp_.OnTick(ms_);

        for(auto &ot : orders_)
        {
			if(ot.isOrdering_){
				GetBestOrderPriceVolume(ot);
				PlaceOrder(ot);
			}
        }
    }
    ms_.UpdateLastTickInfo(md);
}

extern long cur_md_nano;
void CLossModel::GetTimeleft(){
	double extra_time = 0.0;
    if(lst_tick_nano_ > 0  && cur_md_nano > lst_tick_nano_ + 10 * 60 * 1000000000L)  // across time break > 10min
    {
    	extra_time += (cur_md_nano - lst_tick_nano_) / 1000000000.0;
    }
    lst_tick_nano_ = cur_md_nano;

	for(auto &eo : orders_)
	{
		if(eo.isOrdering_)
		{
			eo.timeleft_ = eo.duration_ - (cur_md_nano/1000000000.0 - eo.order_placetime_) + extra_time;
			if(eo.timeleft_ >= valid_duration_){
				eo.timeleft_ = valid_duration_;
			}
			LOG_LVL(DEBUGLEVEL0, "ExecutionOrder %d: timeleft: %lf", eo.order_id_, eo.timeleft_);
		}
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

int CLossModel::OnExecution(double priceup, double pricedown, int placedsize, int direction, double duration){ // for higher level strategy usage
	orders_.emplace_back();
	auto &ot = orders_.back();
    ot.isOrdering_ = true;
    ot.priceup_ = (int)round(priceup / ticksize_);
    ot.pricedown_ = (int)round(pricedown / ticksize_);
    ot.totalsize_ = placedsize;
    ot.direction_ = direction == AT_CHAR_Buy ? 1 : -1;
    ot.duration_  = duration;
    ot.cancel_status_ = CANCEL_STATUS_NORMAL;
    ot.order_placetime_ = CTimer::instance().getNano()/1000000000.0;
    ot.timeleft_ = valid_duration_;
    int ret_id = orders_.size() - 1;
    ot.order_id_ = ret_id;
    LOG_LVL(DEBUGLEVEL1, "%s|OnExecution %d: priceup:%lf, pricedown:%lf", parseNano(cur_md_nano, "%Y%m%d-%H:%M:%S").c_str(), ret_id, priceup, pricedown);
    GetBestOrderPriceVolume(ot);
    PlaceOrder(ot);
    return ret_id;
}

