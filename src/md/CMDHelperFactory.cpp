/*
 * CMDHelperFactory.cpp
 *
 *  Created on: 2018年5月9日
 *      Author: sky
 */

#include "CMDHelperFactory.h"

#include "CMDHelperComm.h"
#include "CMDHelperPipe.h"
#include "CMDHelperPython.h"
#include "CMDHelperReplayCtpDump.h"
#include "CMDHelperReplayIO.h"

IMDHelper* CMDHelperFactory::create(string type, string name) {
  if (type == "comm") {
    return new CMDHelperComm(name);
  } else if (type == "replay_mmap") {
    return new CMDHelperReplayIO(name);
  } else if (type == "replay_ctpdump") {
    return new CMDHelperReplayCtpDump(name);
  } else if (type == "pipe") {
    return new CMDHelperPipe(name);
  } else if (type == "python") {
    return new CMDHelperPython(name);
  }
  return nullptr;
}
