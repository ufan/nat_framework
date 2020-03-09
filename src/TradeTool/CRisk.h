/*
 * CRisk.h
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CRISK_H_
#define SRC_TRADER_CRISK_H_

#include <string>
using namespace std;

class CRisk
{
public:
	CRisk();
	virtual ~CRisk();

	bool check(string instrument, double price, int volume, int direction, int offset);

	bool checkOpenAuth();

private:
	bool has_open_auth_;
};

#endif /* SRC_TRADER_CRISK_H_ */
