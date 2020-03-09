/*
 * testpython.cpp
 *
 *  Created on: 2018年2月26日
 *      Author: hongxu
 */

#include <iostream>
#include <vector>
#include <string>
#include <boost/python.hpp>
using namespace boost::python;
using namespace std;


list getModuleList()
{
	list tmp;
	tmp.append("1");
	tmp.append(2);
	tmp.append(3.4);

	list result;
	result.append("list");
	result.append(tmp);
	result.append(6);

	return result;
}

void testList(vector<string> b)
{
	b.push_back("hello");
	for(auto i: b)
	{
		cout << i << endl;
	}
}

BOOST_PYTHON_MODULE(testpython)
{
	def("getModuleList", getModuleList);
	def("testList", testList);
}

