/*
 * test.cpp
 *
 *  Created on: Sep 21, 2017
 *      Author: hongxu
 */

#include "CConfig.h"
#include <stdint.h>
#include <iostream>
using namespace std;


void test()
{
	CConfig config("./test.cnf");

	string s = config.getVal<string>("BLOCK1", "string_key");
	cout << s << endl;

	double d = config.getVal<double>("BLOCK2", "double_key");
	cout << d << endl;

	uint64_t u = config.getVal<uint64_t>("BLOCK2", "uint_key");
	cout << u << endl;

	int i = config.getVal<int>("BLOCK1", "int_key");
	cout << i << endl;

	vector<string> v;
	config.getValList("BLOCK1", "list_key", v);
	for(vector<string>::iterator itr = v.begin(); itr != v.end(); itr++)
	{
		cout << *itr << ";";
	}
	cout << endl;

	double deflt = config.getVal("BLOCK2", "default_key", 2193.2051);
	cout << deflt << endl;

	deflt = config.getVal("BLOCK3", "double_key", 8471.641);
	cout << deflt << endl;

	try
	{
		double noexist = config.getVal<int>("NOEXIST", "none_key");
		cout << noexist << endl;
	}
	catch(CConfig::KeyNotFound &e)
	{
		cerr << "Key " << e.key << " not found." << endl;
	}

	vector<int> vint;
	config.getList("BLOCK2", "list", vint);
	for(auto &itr : vint)
	{
		cout << itr << "|";
	}
	cout << endl;

	vint.clear();
	config.getList("BLOCK2", "list2", vint, " ,");
	for(auto &itr : vint)
	{
		cout << itr << "|";
	}
	cout << endl;
}



int main()
{
	test();

	return 0;
}

