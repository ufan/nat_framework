/*
 * CClient.h
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#ifndef SRC_TA_CCLIENT_H_
#define SRC_TA_CCLIENT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "CCrypt.h"
#include "CConfig.h"
#include "Logger.h"
using namespace std;

class CClient
{
public:
	CClient();
	virtual ~CClient();

	bool connect(string ip, uint16_t port);

	bool sslAuthen();

	bool authentication();

	bool init(string cfg);

	void run(vector<string> &extr_cmd);

	bool sendDesKey();

	bool desSend(string pkg);

	bool desRead(string &pkg);

	bool doCommand(vector<string> &extr_cmd);

	bool doOneCommand(string cmd);

	bool loopSendFile(int fd, string &data);

	bool saveFile(string file, string dst);

	bool doShellCmd(string cmd);

	bool doAssertCmd(string target);

	bool loopReadAndEcho();

	int readEcho();

	bool sendFile(string file, string name, uint8_t cmd, uint8_t type=0);

	bool sendDir(string localdir, string remotedir, uint8_t cmd, uint8_t type=0);

	bool doExecCmd(string config, string execfile, string argstr, string workdir=string());

	bool doStgCmd(vector<string> cmd);

	bool sayBye();

	static CConfig* getConfig();

private:
	int 			fd_;
	CCrypt 		crypter_;
	string 		last_shell_output_;
};

#endif /* SRC_TA_CCLIENT_H_ */
