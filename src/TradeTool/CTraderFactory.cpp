/*
 * CTraderFactory.cpp
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#include "CTraderFactory.h"
#include "CCtpTrader.h"
#include "CCtpMiniTrader.h"
#include "TraderLogger.h"
#include "CEESTrader.h"

CTraderFactory::CTraderFactory()
{

}

CTraderFactory::~CTraderFactory()
{

}

CTraderBase* CTraderFactory::getTrader(string desk)
{
	CTraderBase *p = NULL;
	if(desk == "ctp")
	{
		p = dynamic_cast<CTraderBase*>(new CCtpTrader);
	}
	else if(desk == "ctpmini")
	{
		p = dynamic_cast<CTraderBase*>(new CCtpMiniTrader);
	}
	else if(desk == "ees")
	{
		p = dynamic_cast<CTraderBase*>(new CEESTrader);
	}
	else
	{
		LOG_ERR("Unknown desk!");
	}

	return p;
}
