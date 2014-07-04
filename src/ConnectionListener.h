#ifndef __CONNECTIONLISTENER__
#define __CONNECTIONLISTENER__
#include "Server.h"

struct ConnectionListener : Task
{
  std::string port;
  ConnectionListener(Server *s, std::string p);
  Command run();
  int getSocket(std::string);
};


#endif
