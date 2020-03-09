/*
 * CLogDB.cpp
 *
 *  Created on: 2018年2月9日
 *      Author: hongxu
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string/trim.hpp>
#include "CLogDB.h"

CLogDB::CLogDB() : has_filter_(false)
{

}

CLogDB::~CLogDB()
{

}

void CLogDB::addFilter(string tbl, vector<string> cols)
{
	auto itr = tbl_parser_.find(tbl);
	if(itr == tbl_parser_.end())
	{
		CLogTable *parser = newLogTable();
		parser->setColFilter(cols);
		tbl_parser_[tbl].reset(parser);
	}
	else
	{
		itr->second->setColFilter(cols);
	}

	has_filter_ = true;
}

bool CLogDB::parseFirstLine(const char *p)
{
	int pos = 0;
	while(*p)
	{
		while(*p == ' ' || *p == '\t') p++;
		if(*p == '\0') break;

		if(pos == 0)
		{
			if(*p != '#') return false;
			p++;
		}
		else
		{
			string token;
			token.push_back(*p);
			while(*++p && *p != ' ' && *p != '\t') token.push_back(*p);

			if(pos == 1 && token != "DBName:") return false;
			else if(pos == 2) db_name_ = token;
			else if(pos == 3)
			{
				version_ = token;
				return true;
			}
		}
		pos ++;
	}
	return false;
}

bool CLogDB::read(string file)
{
	ifstream in(file.c_str());
	if(!in) return false;

	bool has_get_db_name_ = false;
	string line;

	while(getline(in, line))
	{
		if(has_get_db_name_)
		{
			const char *p = line.c_str();
			while(*p == ' ' || *p == '\t') p++;
			if(*p == '#')
			{
				while(*++p == ' ' || *p == '\t');
				if(*p == '\0') continue;

				string token;
				while(*p && *p !=' ' && *p != '\t') token.push_back(*p++);
				if(*(p-1) != ':') continue;
				token.resize(token.size() - 1);

				if(!has_filter_)
				{
					CLogTable *parser = newLogTable();
					if(!parser->parseDescInfo(line.c_str()))
					{
						delete parser;
						continue;
					}
					tbl_parser_[token].reset(parser);
				}
				else
				{
					auto itr = tbl_parser_.find(token);
					if(itr != tbl_parser_.end())
					{
						if(!itr->second->parseDescInfo(line.c_str()))
						{
							itr->second->clear();
							continue;
						}
					}
				}
			}
			else if(*p)
			{
				string token;
				while(*p && *p !=' ' && *p != '\t') token.push_back(*p++);
				auto itr = tbl_parser_.find(token);
				if(itr != tbl_parser_.end())
				{
					itr->second->parseLine(p);
				}
			}
		}
		else
		{
			const char *p = line.c_str();
			while(*p == ' ' || *p == '\t') p++;
			if(!parseFirstLine(p)) return false;
			has_get_db_name_ = true;
		}
	}

	in.close();
	return true;
}

vector<string> CLogDB::getTableNames()
{
	vector<string> tables;
	for(auto itr : tbl_parser_)
	{
		tables.emplace_back(itr.first);
	}
	return tables;
}

