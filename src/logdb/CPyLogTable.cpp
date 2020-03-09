/*
 * CPyLogTable.cpp
 *
 *  Created on: 2018年2月26日
 *      Author: hongxu
 */

#include "CPyLogTable.h"

CPyLogTable::CPyLogTable()
{

}

CPyLogTable::~CPyLogTable()
{

}

bool CPyLogTable::parseLine(const char *p)
{
	int idx_pos = 0;
	int pos = 0;

	if(select_col_idx_.size())
	{
		while(*p == ' ' || *p == '\t') p++;   // skip blank after table name

		list line;
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
				case COL_TYPE_STRING: line.append(token); break;
				case COL_TYPE_INT:
				{
					int64_t val = strtoll(token, NULL, 0);
					line.append(val);
					break;
				}
				case COL_TYPE_DOUBLE:
				{
					double val = strtod(token, NULL);
					line.append(val);
					break;
				}
				default: ;
				}

				*(char*)p = c;
				if(c == '|') p++;

				if(++idx_pos == select_col_idx_.size())
				{
					values_.append(line);
					return true;
				}
			}
			else
			{
				while(*++p && *p !='|');
				if(*p == '|') p++;
			}
			pos++;
		}
	}

	return false;
}

