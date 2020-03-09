/*
 * CSaveFileCmd.h
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#ifndef SRC_TA_CSAVEFILECMD_H_
#define SRC_TA_CSAVEFILECMD_H_

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "CCommanderBase.h"

class CSaveFileCmd: public CCommanderBase
{
#pragma pack(1)
	struct tHead
	{
		char 		digest[32];					// SHA256 digest
		struct stat file_st;
		char 		name_and_content[0];
	};
#pragma pack()

public:
	CSaveFileCmd(CWaiter *p_owner);
	virtual ~CSaveFileCmd();

	virtual int run(string &pkg);

private:
	int processFirstPkg(string &pkg);
	int processPkg(string &pkg);

	bool checkDigest();

private:
	bool		is_fist_pkg_;
	int			fd_;
	uint32_t	left_len_;
	string 		name_;
	char 		digest_[32];
};

#endif /* SRC_TA_CSAVEFILECMD_H_ */
