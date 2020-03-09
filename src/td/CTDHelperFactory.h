/*
 * CTDHelperFactory.h
 *
 *  Created on: 2018年5月10日
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDHELPERFACTORY_H_
#define SRC_TD_CTDHELPERFACTORY_H_

#include <string>
#include "ITDHelper.h"
using namespace std;


class CTDHelperFactory
{
public:
	static ITDHelper* create(string type, string name);
};

#endif /* SRC_TD_CTDHELPERFACTORY_H_ */
