/*
 * DeskTrade.cpp
 *
 *  Created on: 2017年12月14日
 *      Author: hongxu
 */

#include <stdio.h>
#include <unistd.h>
#include <map>
#include <string>
#include "CGlobalParameter.h"
#include "CTraderFactory.h"
#include "EnumStatus.h"
#include "CRisk.h"
using namespace std;

static volatile bool g_run = true;
static CTraderBase *g_trader = NULL;

void doExit();

int queryAccount()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryAccount();
	if(0 != ret)
	{
		cerr << "QryAccount failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

int queryPosition()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryPosition();
	if(0 != ret)
	{
		cerr << "QryPosition failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

int queryOrder()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryOrder();
	if(0 != ret)
	{
		cerr << "QryOrder failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

int queryTrade()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryTrade();
	if(0 != ret)
	{
		cerr << "QryTrade failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

int queryTradeMargin()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryTradeMargin();
	if(0 != ret)
	{
		cerr << "QryTradeMargin failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

int queryTradeFee()
{
	CGlobalParameter::terminal_lock_ = true;
	int ret = g_trader->qryTradeFee();
	if(0 != ret)
	{
		cerr << "QryTradeFee failed!" << endl;
		CGlobalParameter::terminal_lock_ = false;
		return ret;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return 0;
}

void parseCmd()
{
	string line;
	stringstream ss;
	CRisk risk;

	while(g_run)
	{
		cout << "<< ";
		getline(cin, line);
		ss.str(line);

		string s;
		ss >> s;

		if(s == "acc")
		{
			queryAccount();
		}
		else if(s == "pos")
		{
			queryPosition();
		}
		else if(s == "sell" || s == "buy")
		{
			OMSDirection dir = s == "sell" ? OD_Sell : OD_Buy;
			ss >> s;
			OMSOffset off = OO_Close;
			if(s == "open")
			{
				off = OO_Open;
			}
			else if(s == "close_td")
			{
				off = OO_CloseToday;
			}
			else if(s == "close")
			{
				off = OO_Close;
			}
			else
			{
				cerr << "unknown offset " << s << endl;
				ss.str(""); ss.clear(); continue;
			}

			string inst;
			ss >> inst;

			double price;
			ss >> price;

			int vol;
			ss >> vol;

			if(!risk.check(inst, price, vol, dir, off))
			{
				cerr << "authentication failed." << endl;
			}
			else if(g_trader->sendOrder(inst, price, vol, dir, off)) cerr << "send order err." << endl;
		}
		else if(s == "order")
		{
			queryOrder();
		}
		else if(s == "trade")
		{
			queryTrade();
		}
		else if(s == "del")
		{
			int id = -1;
			ss >> id;
			if(id < 0)
			{
				cerr << "err, usage: del order_id" << endl;
			}
			else
			{
				map<int, string>::iterator itr = CGlobalParameter::order_id_map_.find(id);
				if(itr == CGlobalParameter::order_id_map_.end())
				{
					cerr << "del err: order " << id << " not found." << endl;
				}
				else g_trader->deleteOrder(itr->second);
			}
		}
		else if(s == "margin")
		{
			queryTradeMargin();
		}
		else if(s == "fee")
		{
			queryTradeFee();
		}
		ss.str(""); ss.clear();
	}
}

bool initDesk(string desk)
{
	g_trader = CTraderFactory::getTrader(desk);
	if(!g_trader) return false;

	CGlobalParameter::terminal_lock_ = true;
	if(!g_trader->init())
	{
		CGlobalParameter::terminal_lock_ = false;
		return false;
	}
	while(CGlobalParameter::terminal_lock_) usleep(25000);
	return true;
}

void usage()
{
	cout << "Usage: TradeTool desk config_file" << endl;
	exit(-1);
}

void doExit()
{
	g_run = false;
}

int main(int argc, char *argv[])
{
	if(argc != 3) usage();

	assert(CGlobalParameter::initConfig(argv[2]));
	assert(initDesk(argv[1]));

	parseCmd();
	return 0;
}

