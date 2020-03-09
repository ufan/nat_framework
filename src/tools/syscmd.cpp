/*
 * syscmd.cpp
 *
 *  Created on: 2019年1月7日
 *      Author: hongxu
 */
#include <iostream>
#include <string>
#include <fstream>
#include "CSystemIO.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "Logger.h"
using namespace std;

const char *g_self_name = "syscmd";
int	g_backwd = 0;

int parseOffset(string off)
{
	if(off == "open") return AT_CHAR_Open;
	else if(off == "close") return AT_CHAR_Close;
	else if(off == "close_td") return AT_CHAR_CloseToday;
	else if(off == "close_yd") return AT_CHAR_CloseYesterday;
	return -1;
}

void addExecOrder()
{
	tIOUserAddExecOrder data;
	string in;

	cout << "to:";
	std::getline(std::cin, in);
	data.to = (int)HASH_STR(in.c_str());
	data.source = (int)HASH_STR(g_self_name);
	data.back_word = g_backwd++;
	data.cmd = IO_USER_ADD_EXEC_ORDER;

	cout << "instr:";
	std::getline(std::cin, in);
	data.instr_hash = INSTR_NAME_TO_HASH(in.c_str());
	cout << "dir(buy or sell):";
	std::getline(std::cin, in);
	data.dir = in == "buy" ? AT_CHAR_Buy : AT_CHAR_Sell;
	cout << "off(open|close|close_td|close_yd):";
	std::getline(std::cin, in);
	data.off = parseOffset(in);
	cout << "price:";
	std::getline(std::cin, in);
	data.price = stod(in);
	cout << "vol:";
	std::getline(std::cin, in);
	data.vol = stoi(in);
	cout << "acc_idx(default 0):";
	std::getline(std::cin, in);
	data.acc_idx = in.empty() ? 0 : stoi(in);
	data.stg_id = 0;

	CSystemIO::instance().getWriter().write(&data, sizeof(data));
	cout << "send command to system complete." << endl;
}

void sendStr()
{
	tSysIOHead head;
	string in;

	cout << "to:";
	std::getline(std::cin, in);

	head.to = (int)HASH_STR(in.c_str());
	head.source = (int)HASH_STR(g_self_name);
	head.back_word = g_backwd++;
	head.cmd = IO_USER_CMD;

	cout << "content:";
	std::getline(std::cin, in);

	string data((const char*)&head, sizeof(head));
	data += in;
	CSystemIO::instance().getWriter().write(data.data(), data.size());
	cout << "send command to system complete." << endl;
}

void sendFile()
{
	tSysIOHead head;
	string in;

	cout << "to:";
	std::getline(std::cin, in);

	head.to = (int)HASH_STR(in.c_str());
	head.source = (int)HASH_STR(g_self_name);
	head.back_word = g_backwd++;
	head.cmd = IO_USER_CMD;

	cout << "file_path:";
	std::getline(std::cin, in);

	ifstream infile(in);
	if(!infile)
	{
		LOG_ERR("read config file %s err.\n", in.c_str());
		return;
	}
	string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
	infile.close();

	string data((const char*)&head, sizeof(head));
	data += content;
	CSystemIO::instance().getWriter().write(data.data(), data.size());
	cout << "send command to system complete." << endl;
}

void run()
{
	while(1)
	{
		string cmd;
		cout << "<< ";
		if(std::getline(std::cin, cmd).eof()) break;

		if(cmd == "add exec order")
		{
			addExecOrder();
		}
		else if(cmd == "help")
		{
			cout << "command list: help | add exec order | str | file" << endl;
		}
		else if(cmd == "str")
		{
			sendStr();
		}
		else if(cmd == "file")
		{
			sendFile();
		}
		else
		{
			cout << "unknown command" << endl;;
		}
	}
}

int main()
{
	initLogger("./logger.cnf");
	if(!CSystemIO::instance().init())
	{
		ALERT("init system io writer err.");
		return -1;
	}

	run();
	return 0;
}

