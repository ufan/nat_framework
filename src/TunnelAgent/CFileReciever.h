/*
 * CFileReciever.h
 *
 *  Created on: 2017年10月2日
 *      Author: hongxu
 */

#ifndef SRC_TA_CFILERECIEVER_H_
#define SRC_TA_CFILERECIEVER_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "CCommanderBase.h"
#include "protocol.h"

class CFileReciever: public CCommanderBase {
protected:
#pragma pack(1)
	struct tFileHead
	{
		char 		digest[32];					// SHA256 digest
		struct stat	file_st;
		char 		name_and_content[0];
	};
#pragma pack()

public:
	CFileReciever(CWaiter *p_owner);
	virtual ~CFileReciever();

	virtual int run(string &pkg);

	virtual int processFirstPkg(string &pkg);

	virtual int processPkg(string &pkg);

	virtual bool checkDigest();

	virtual string& getContent() {return content_;}

	virtual string getName() {return name_;}

	virtual struct stat getFileStat() {return file_st_;}

protected:
	bool			is_first_pkg_;
	uint32_t		left_len_;
	string			name_;
	string			content_;
	struct stat		file_st_;
	char 			digest_[32];
};

#endif /* SRC_TA_CFILERECIEVER_H_ */
