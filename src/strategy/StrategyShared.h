/*
 * CStrategyShared.h
 *
 *  Created on: May 22, 2018
 *      Author: hongxu
 */

#ifndef SRC_STRATEGY_STRATEGYSHARED_H_
#define SRC_STRATEGY_STRATEGYSHARED_H_

#include <string>
#include "strategy_shared_comm.h"
using namespace std;

bool initStrategyShared(string name);

string getStrategyName();

tStrategyNode* getSharedData();

void setStrategyConfig(string content);

string getStrategyConfig();

bool registerStg(string name);

bool delStg(string name);

#endif /* SRC_STRATEGY_STRATEGYSHARED_H_ */
