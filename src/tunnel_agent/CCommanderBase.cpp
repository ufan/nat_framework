/*
 * CCommanderBase.cpp
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#include "CCommanderBase.h"
#include "CWaiter.h"

CCommanderBase::CCommanderBase(CWaiter *p_owner) : p_owner_(p_owner)
{

}

CCommanderBase::~CCommanderBase()
{

}

void CCommanderBase::sendToClient(string& content)
{
	string s;
	s.push_back((char)CMD_ALLDATA);
	s += content;
	p_owner_->desSendData(s);
}
