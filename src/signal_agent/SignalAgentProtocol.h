/*
 * SignalAgentProtocol.h
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 */

#ifndef SRC_SIGNAL_AGENT_SIGNALAGENTPROTOCOL_H_
#define SRC_SIGNAL_AGENT_SIGNALAGENTPROTOCOL_H_

#include <stdint.h>

#define SA_STX 0x88
#define SA_VER 1

#pragma pack(1)

struct tSACmdHead
{
	uint8_t stx = SA_STX;
	uint8_t ver = SA_VER;
	uint8_t	cmd = 0;
	int 	len = 0;
	char	buf[0];
};

struct tSARequestSignal : tSACmdHead
{
	int 	sig_cnt = 0;
	char	sig_names[0];
};

struct tSASignalData : tSACmdHead
{
	int  sig_idx;
	char sig_data[0];
};

enum emSACmd
{
	SACMD_REQ_SIG = 1,
	SACMD_SIGNAL_DATA,
	SACMD_REQ_HEATBEAT,
	SACMD_REP_HEATBEAT,
};

#pragma pack()

#define CHECK_SA_HEAD(p) ((p)->stx == SA_STX && (p)->ver == SA_VER)


#endif /* SRC_SIGNAL_AGENT_SIGNALAGENTPROTOCOL_H_ */
