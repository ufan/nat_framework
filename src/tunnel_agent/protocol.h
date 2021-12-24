/*
 * protocol.h
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#ifndef SRC_TA_PROTOCOL_H_
#define SRC_TA_PROTOCOL_H_

#include <stdint.h>

#define META_HEAD_STX 0x68
#define META_HEAD_VER 0x01

#define MAX_DATA_LEN (100 * 1024 * 1024)  // 100M

// all the network structure are in host byte order
#pragma pack(1)
struct tMetaHead {
  uint8_t stx;
  uint8_t ver;
  uint16_t reserved;
  uint32_t len;
};

struct tSSLAuthenBody {
  char rand[64];
  uint8_t encrypted[0];
};

enum emCommand {
  CMD_SAVEFILE,  // client -> agent : send file
  CMD_SHELL,     // client -> agent : shell command
  CMD_PYTHON,
  CMD_EXEC,    // client -> python script strategy
  CMD_FINISH,  // agent -> client : command finished
  CMD_STG,     // client -> agent : strategy management
  CMD_BYE,     // client -> agent : no more commands, session end

  CMD_ALLDATA,  // agent -> client : data sent from client relating to the
                // running command
};

enum emCmdType {
  TYPE_FINISH_SUCC,
  TYPE_FINISH_FAIL,
  TYPE_FINISH_ACK,
};

struct tCommand {
  uint8_t cmd;
  uint8_t type;
  uint32_t content_len;
};

enum emStgType {
  STG_STATUS,
  STG_START_TRADE,
  STG_STOP_TRADE,
  STG_EXIT,
  STG_SIGNAL,
  STG_SET_DATA,
};

struct tStgCommand {
  uint8_t cmd;
  uint8_t type;
  uint64_t data;
  char name[0];
};

#pragma pack()

// common states for state machine
enum emState {
  STATE_PENDING,  // wait for request data
  STATE_NEXT,     // goto next action
  STATE_ABORT,    // abort session
  STATE_FINISH,   // finish session
};

#endif /* SRC_TA_PROTOCOL_H_ */
