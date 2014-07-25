#ifndef __CONSOLD__
#define __CONSOLD__
#include "Server.h"
#include <map>
#include <vector>

struct Consold : Task
{
  std::string line;
  Consold(Server *);
  typedef std::vector<std::string> Args;
  typedef Command (Consold::*action)(Args &args);
  std::map<std::string,action> command;
  message getReply();
  Command run();
  void prompt();
  void prog6620(unsigned char prog[267][9]);
  Command doBaseline(Args &args);
  Command doCircularity(Args &args);
  Command doFrequency(Args &args);
  Command doPan(Args &args);
  Command doQuit(Args &args);
  Command doStatus(Args &args);
  Command doFirmwareVersion(Args &args);
  Command doBootVersion(Args &args);
  Command doRun(Args &args);
  Command doStop(Args &args);
  Command doRFGain(Args &args);
  Command doIFGain(Args &args);
  Command doOutput(Args &args);
  Command doBandwidth(Args &args);
  Command doSwitch(Args &args);
  //Command doSampleRate(Args &args);
};


#endif
