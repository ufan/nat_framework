/*
 * CGlobalParamter.cpp
 *
 *  Created on: 2017年12月20日
 *      Author: hongxu
 */

#include "CGlobalParameter.h"

CConfig CGlobalParameter::cnf_;
volatile bool CGlobalParameter::terminal_lock_ = false;
map<int, string> CGlobalParameter::order_id_map_;

CGlobalParameter::CGlobalParameter()
{

}

CGlobalParameter::~CGlobalParameter()
{

}

