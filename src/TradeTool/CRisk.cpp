/*
 * CRisk.cpp
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#include <iostream>
#include "CRisk.h"
#include "EnumStatus.h"

CRisk::CRisk() : has_open_auth_(false)
{

}

CRisk::~CRisk()
{

}

bool CRisk::check(string instrument, double price, int volume, int direction,
		int offset)
{
	if(OO_Open == offset)
	{
		if(!checkOpenAuth()) return false;
	}

	return true;
}

bool CRisk::checkOpenAuth()
{
	if(has_open_auth_) return true;

	cout << "please input code for authentication: ";
	string code;
	getline(cin, code);

	if(code == "888888")
	{
		has_open_auth_ = true;
		return true;
	}

	cerr << "Error code, please contact manager." << endl;
	return false;
}

