#include "CLogitPModel.h"
#include "PublicFun.h"
#include "math.h"
#include <fstream>
#include "CTimer.h"
#include "Logger.h"


CLogitPModel::CLogitPModel(){
    startsize_ = 1;
    endsize_ = 50;
    
    for(int deltacount = 0; deltacount < TOTAL_DELTA_; deltacount++){
        buy_startprob_logskewCoeff_[deltacount] = 0;
        buy_endprob_logskewCoeff_[deltacount] = 0;
        buy_startprob_intercept_[deltacount] = 0;
        buy_endprob_intercept_[deltacount] = 0;
        for(int hcount =0; hcount< HORIZON_USE_; hcount++){
            buy_startprob_tradecoeff_[deltacount][hcount] = 0;
            buy_endprob_tradecoeff_[deltacount][hcount] = 0;
            buy_startprob_bookcoeff_[deltacount][hcount] = 0;
            buy_endprob_bookcoeff_[deltacount][hcount] = 0;
            buy_startprob_OBDcoeff_[deltacount][hcount] = 0;
            buy_endprob_OBDcoeff_[deltacount][hcount] = 0;
        }
    }
    
    for(int deltacount = 0; deltacount < TOTAL_DELTA_; deltacount++){
        sell_startprob_logskewCoeff_[deltacount] = 0;
        sell_endprob_logskewCoeff_[deltacount] = 0;
        sell_startprob_intercept_[deltacount] = 0;
        sell_endprob_intercept_[deltacount] = 0;
        for(int hcount =0; hcount< HORIZON_USE_; hcount++){
            sell_startprob_tradecoeff_[deltacount][hcount] = 0;
            sell_endprob_tradecoeff_[deltacount][hcount] = 0;
            sell_startprob_bookcoeff_[deltacount][hcount] = 0;
            sell_endprob_bookcoeff_[deltacount][hcount] = 0;
            sell_startprob_OBDcoeff_[deltacount][hcount] = 0;
            sell_endprob_OBDcoeff_[deltacount][hcount] = 0;
        }
    }
}

CLogitPModel::~CLogitPModel()
{}

bool CLogitPModel::init(string config)
{
    std::ifstream f1(config);
    if(!f1)
    {
    	LOG_ERR("open %s failed.", config.c_str());
    	return false;
    }

    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();

    json param_reader = json::parse(content);
    startsize_ = param_reader["startSize"];
    endsize_ = param_reader["endSize"];
    
    int DELTA[TOTAL_DELTA_];    
    for(int count=0; count< TOTAL_DELTA_; count++){
        DELTA[count] = param_reader["Delta"][count];
        IsProbSet1_[count] = param_reader["IsProbSet1"][std::to_string(DELTA[count])];
    }
    
    for(int count = 0; count < TOTAL_DELTA_; count++){
        int k =0;        
        buy_startprob_intercept_[count] = param_reader["startBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        buy_endprob_intercept_[count] = param_reader["endBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_startprob_intercept_[count] = param_reader["startSellLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_endprob_intercept_[count] = param_reader["endSellLogitCoeff"][std::to_string(DELTA[count])][k];
        
        k++;
        buy_startprob_logskewCoeff_[count] = param_reader["startBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        buy_endprob_logskewCoeff_[count] = param_reader["endBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_startprob_logskewCoeff_[count] = param_reader["startSellLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_endprob_logskewCoeff_[count] = param_reader["endSellLogitCoeff"][std::to_string(DELTA[count])][k];
        
        for(int hcount =0; hcount< HORIZON_USE_; hcount++){
            k++;
            buy_startprob_tradecoeff_[count][hcount] = param_reader["startBuyLogitCoeff"][std::to_string(DELTA[count])][k];
            buy_endprob_tradecoeff_[count][hcount]   = param_reader["endBuyLogitCoeff"][std::to_string(DELTA[count])][k];
            sell_startprob_tradecoeff_[count][hcount] = param_reader["startSellLogitCoeff"][std::to_string(DELTA[count])][k];
            sell_endprob_tradecoeff_[count][hcount]   = param_reader["endSellLogitCoeff"][std::to_string(DELTA[count])][k];
        }
        
        for(int hcount =0; hcount< HORIZON_USE_; hcount++){
            k++;
            buy_startprob_bookcoeff_[count][hcount] = param_reader["startBuyLogitCoeff"][std::to_string(DELTA[count])][k];
            buy_endprob_bookcoeff_[count][hcount]   = param_reader["endBuyLogitCoeff"][std::to_string(DELTA[count])][k];
            sell_startprob_bookcoeff_[count][hcount] = param_reader["startSellLogitCoeff"][std::to_string(DELTA[count])][k];
            sell_endprob_bookcoeff_[count][hcount]   = param_reader["endSellLogitCoeff"][std::to_string(DELTA[count])][k];
        }
        /*k++;
        buy_startprob_sigmacoeff_[count]  = param_reader["startBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        buy_endprob_sigmacoeff_[count]    = param_reader["endBuyLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_startprob_sigmacoeff_[count] = param_reader["startSellLogitCoeff"][std::to_string(DELTA[count])][k];
        sell_endprob_sigmacoeff_[count]  = param_reader["endSellLogitCoeff"][std::to_string(DELTA[count])][k];  */
    }
    return true;
}

double CLogitPModel::EvaluateProb(int level_index, int tradesize, const CBaseSignal& bs, double timeleft, double duration, int direction){
    double prob = 1.0;
    if(IsProbSet1_[level_index] < 1){
        double buy_startKernel = bs.GetMut(buy_startprob_logskewCoeff_[level_index], buy_startprob_tradecoeff_[level_index], buy_startprob_bookcoeff_[level_index], buy_startprob_OBDcoeff_[level_index])
                               + buy_startprob_intercept_[level_index]; // + volatility * buy_startprob_sigmacoeff_[level_index];
                               
        double buy_endKernel   = bs.GetMut(buy_endprob_logskewCoeff_[level_index], buy_endprob_tradecoeff_[level_index], buy_endprob_bookcoeff_[level_index], buy_endprob_OBDcoeff_[level_index])
                               + buy_endprob_intercept_[level_index]; // + volatility * buy_endprob_sigmacoeff_[level_index];
                               
        double sell_startKernel = bs.GetMut(sell_startprob_logskewCoeff_[level_index], sell_startprob_tradecoeff_[level_index], sell_startprob_bookcoeff_[level_index], sell_startprob_OBDcoeff_[level_index])
                               + sell_startprob_intercept_[level_index]; // + volatility * sell_startprob_sigmacoeff_[level_index];
                               
        double sell_endKernel   = bs.GetMut(sell_endprob_logskewCoeff_[level_index], sell_endprob_tradecoeff_[level_index], sell_endprob_bookcoeff_[level_index], sell_endprob_OBDcoeff_[level_index])
                               + sell_endprob_intercept_[level_index]; // + volatility * sell_endprob_sigmacoeff_[level_index];
        
        buy_startlogit_prob_ = logitProb(buy_startKernel);
        buy_endlogit_prob_   = logitProb(buy_endKernel);
        sell_startlogit_prob_ = logitProb(sell_startKernel);
        sell_endlogit_prob_   = logitProb(sell_endKernel);
        
        buy_startlogit_prob_ = buy_startlogit_prob_ * sqrt(abs(timeleft/duration));
        buy_endlogit_prob_   = buy_endlogit_prob_ * sqrt(abs(timeleft/duration));
        sell_startlogit_prob_ = sell_startlogit_prob_ * sqrt(abs(timeleft/duration));
        sell_endlogit_prob_   = sell_endlogit_prob_ * sqrt(abs(timeleft/duration));
        
        if(direction >0){
            prob = LogLinearInterpolate(startsize_, buy_startlogit_prob_, endsize_, buy_endlogit_prob_, tradesize);
        }
        else{
            prob = LogLinearInterpolate(startsize_, sell_startlogit_prob_, endsize_, sell_endlogit_prob_, tradesize);
        }
    }
        
    return prob;
}

double CLogitPModel::logitProb(double kernel){
    double prob = 1.0/(1.0 + exp(-kernel));
    if(abs(kernel) < 0.000001){
        prob = 0;
    }
    
    return prob;
}


/*    if(direction > 0){
//        string log = std::to_string(prob) + ","  + std::to_string(level_index) + ","  + std::to_string(buy_startKernel)+ "," 
//                   + std::to_string(buy_endKernel) + ","+ std::to_string(buy_startlogit_prob_) + "," 
//                   + std::to_string(buy_endlogit_prob_) + "," + std::to_string(startsize_) + "," + std::to_string(endsize_) + ","
//                   + std::to_string(timeleft);
        string log = std::to_string(prob) + ","  + std::to_string(level_index) + ","
                   + std::to_string(buy_endKernel) + "," + std::to_string(buy_endlogit_prob_) 
                   + "," + std::to_string(startsize_) + "," + std::to_string(endsize_) + "," + std::to_string(buy_endprob_intercept_[level_index]) 
                   + "," + std::to_string(buy_endprob_logskewCoeff_[level_index]) + "," + std::to_string(buy_endprob_tradecoeff_[level_index][0]) 
                   + "," + std::to_string(buy_endprob_tradecoeff_[level_index][1]) + "," + std::to_string(buy_endprob_tradecoeff_[level_index][2])
                   + "," + std::to_string(buy_endprob_bookcoeff_[level_index][0]) + "," + std::to_string(buy_endprob_bookcoeff_[level_index][1]) 
                   + "," + std::to_string(buy_endprob_bookcoeff_[level_index][2]) + "," 
                   + std::to_string(timeleft);
                
        fastLog(log_id, log);
    } */
