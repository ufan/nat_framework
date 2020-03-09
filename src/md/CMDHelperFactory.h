/*
 * CMDHelperFactory.h
 *
 *  Created on: 2018年5月9日
 *      Author: sky
 */

#ifndef MD_CMDHELPERFACTORY_H_
#define MD_CMDHELPERFACTORY_H_

#include <string>
#include "IMDHelper.h"
using namespace std;

class CMDHelperFactory
{
public:
	static IMDHelper* create(string type, string name);
};

#endif /* MD_CMDHELPERFACTORY_H_ */
