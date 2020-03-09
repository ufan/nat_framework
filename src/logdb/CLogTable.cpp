/*
 * CLogTable.cpp
 *
 *  Created on: 2018年2月9日
 *      Author: hongxu
 */

#include "string.h"
#include <boost/algorithm/string/trim.hpp>
#include "CLogTable.h"

CLogTable::CLogTable() : is_all_cols_(true)
{

}

CLogTable::~CLogTable()
{

}

void CLogTable::setColFilter(vector<string>& cols)
{
	select_col_map_.clear();
	for(auto itr : cols)
	{
		select_col_map_[itr] = 0;
	}
	is_all_cols_ = false;
}

bool CLogTable::parseDescInfo(const char *p)
{
	int pos = 0;
	while(*p)
	{
		while(*p == ' ' || *p == '\t') p++;
		if(*p == '\0') break;

		pos++;
		if(pos == 1)
		{
			if(*p != '#') return false;
			p++;
		}
		else if(pos == 2)
		{
			tbl_name_.push_back(*p);
			while(*++p && *p !=' ' && *p != '\t') tbl_name_.push_back(*p);
			if(*(p-1) != ':') return false;
			tbl_name_.resize(tbl_name_.size() - 1);
			if(tbl_name_.empty()) return false;
		}
		else
		{
			string token;
			token.push_back(*p);
			while(*++p && *p !=' ' && *p != '\t' && *p != '(' && *p != ')') token.push_back(*p);

			if(is_all_cols_ || select_col_map_.find(token) != select_col_map_.end())
			{
				select_col_idx_.emplace_back(pos - 3);
				select_col_name_.emplace_back(token);
				select_col_map_[token] = select_col_name_.size() - 1;

				string type;
				if(*p == '(')	// parse type
				{
					while(*++p && *p !=' ' && *p != '\t' && *p != ')') type.push_back(*p);
					if(*p++ != ')') return false;
				}
				else if(*p == ')') return false;

				if(type.empty() || type == "string") { select_col_types_.emplace_back(COL_TYPE_STRING); }
				else if(type == "int") { select_col_types_.emplace_back(COL_TYPE_INT); }
				else if(type == "double") { select_col_types_.emplace_back(COL_TYPE_DOUBLE); }
				else return false;
			}
			else
			{
				while(*p && *p !=' ' && *p != '\t') p++;
			}
		}
	}

	if(pos <= 2) return false;
	return true;
}

// please skip the table name first
bool CLogTable::parseLine(const char *p)
{
	int idx_pos = 0;
	int pos = 0;

	if(select_col_idx_.size())
	{
		while(*p == ' ' || *p == '\t') p++;   // skip blank after table name
		while(*p)
		{
			if(pos == select_col_idx_[idx_pos])
			{
				const char *token = p;
				while(*++p && *p !='|');
				char c = *p;
				*(char*)p = '\0';

				switch(select_col_types_[idx_pos])
				{
				case COL_TYPE_STRING: select_col_value_.emplace_back(token); break;
				case COL_TYPE_INT:
				{
					int64_t val = strtoll(token, NULL, 0);
					select_col_value_.emplace_back(string((const char*)&val, sizeof(val)));
					break;
				}
				case COL_TYPE_DOUBLE:
				{
					double val = strtod(token, NULL);
					select_col_value_.emplace_back(string((const char*)&val, sizeof(val)));
					break;
				}
				default: ;
				}

				*(char*)p = c;
				if(c == '|') p++;

				if(++idx_pos == select_col_idx_.size()) return true;
			}
			else
			{
				while(*++p && *p !='|');
				if(*p == '|') p++;
			}
			pos++;
		}
	}

	// revert
	select_col_value_.resize(select_col_value_.size() - idx_pos);
	return false;
}

