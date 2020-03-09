/*
 * CPyLogDB.cpp
 *
 *  Created on: 2018年2月26日
 *      Author: hongxu
 */

#include "CPyLogDB.h"

CPyLogDB::CPyLogDB()
{

}

CPyLogDB::~CPyLogDB()
{

}

BOOST_PYTHON_MODULE(LogDB)
{
    class_<CPyLogTable>("LogTable")
        .def("getColNames", &CPyLogTable::getColNames)
        .def("getColTypes", &CPyLogTable::getColTypes)
		.def("getTableName", &CPyLogTable::getTableName)
		.def("getValues", &CPyLogTable::getValues)
    ;

    class_<CPyLogDB>("LogDB")
        .def("addFilter", &CPyLogDB::addFilter)
        .def("read", &CPyLogDB::read)
        .def("getTable", &CPyLogDB::getTable, return_value_policy<reference_existing_object>())
		.def("getTableList", &CPyLogDB::getTableList)
		.def("getDBName", &CPyLogDB::getDBName)
		.def("getVer", &CPyLogDB::getVer)
    ;
}

