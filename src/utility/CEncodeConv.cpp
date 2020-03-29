/*
 * CEncodeConv.cpp
 *
 *  Created on: 2018年1月5日
 *      Author: hongxu
 */

#include "CEncodeConv.h"
#include <iconv.h>
#include <string.h>


CEncodeConv::CEncodeConv()
{

}

CEncodeConv::~CEncodeConv()
{

}

string CEncodeConv::conv(string str, string from_charset, string to_charset)
{
	iconv_t cd = iconv_open(to_charset.c_str(), from_charset.c_str());
	if (cd == 0)
	{
		return str;
	}

	char *pstr = (char*)str.c_str();
	char **pin = &pstr;
	size_t inlen = str.size();

	char outbuf[2048];
	char *pbuf = outbuf;
	char **pout = &pbuf;
	size_t outlen = sizeof(outbuf);

	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
	{
		iconv_close(cd);
		return str;
	}
	iconv_close(cd);
	return string(outbuf, sizeof(outbuf) - outlen);
}
