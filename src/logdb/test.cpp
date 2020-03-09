/*
 * test.cpp
 *
 *  Created on: 2018年2月12日
 *      Author: hongxu
 */

#include <iostream>
#include <string>
#include "CLogDB.h"
using namespace std;

string getReadableString(const char *p, int type)
{
	string res;
	switch(type)
	{
	case COL_TYPE_STRING: res = p; break;
	case COL_TYPE_INT:
	{
		char b[20];
		sprintf(b, "%lld", *(long long*)p);
		res = b;
		break;
	}
	case COL_TYPE_DOUBLE:
	{
		char b[20];
		sprintf(b, "%lf", *(double*)p);
		res = b;
		break;
	}
	default: res = "Unknown";
	}
	return res;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s file\n", argv[0]);
		return -1;
	}

	CLogDB db;
	db.read(argv[1]);

	CLogTable *p = db.getLogTable("TestV1");
	if(!p)
	{
		cout << "cannot find table" << endl;
		return -1;
	}
	cout << "Tbl Name: " << p->getTableName() << endl;
	vector<string> col_names = p->getColNames();
	for(auto itr : col_names)
	{
		cout << itr << " ";
	}
	cout << endl;

	vector<int> col_types = p->getColTypes();
	int line_cnt = p->getLineCount();
	for(int i = 0; i < line_cnt; i++)
	{
		for(int j = 0; j < col_names.size(); j++)
		{
			const char *pb = p->getValBuffer(i, j);
			string str = getReadableString(pb, col_types[j]);
			cout << str << " ";
		}
		cout << endl;
	}

	string str = getReadableString(p->getValBuffer(3, "fourth"), p->getColType("fourth"));
	cout << "3|fourth: " << str << endl;
}


