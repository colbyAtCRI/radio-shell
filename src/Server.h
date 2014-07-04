#ifndef __MYSERVER__
#define __MYSERVER__
#include "SdrIQ.h"
#include <list>
#include <string>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

enum Command {
  QUIT,
  CONTINUE,
};

struct Server;

// who'd a tunk it
struct Task
{
  int     device;
  Server *server;
  Task();
  Task(Server *);
  virtual Command run() {return QUIT; }
};

typedef Task *pTask;


// We make the server a task which 
// monitors reads from the radio. When
// the radio collects data it is sent
// as a datagram to data_socket as a
// datagram.
struct Server : std::list<pTask>, Task
{
  unsigned long    ndata;
  int              data_socket;
  struct addrinfo *info;
  FILE            *outp;

  Server(std::string name);

  bool    getDataSocket(std::string port);
  void    add(pTask tsk);
  Command run();
  int     loop();
  void    broadcast(message msg);
  bool    send(message msg);
  message recv();

  void    setFrequency(int freq);
  double  getFrequency();
  void    setIFGain(int g);
  int     getIFGain();
  void    setFilterIndex(int n);
  std::string  getName();
  double  getFirmwareVersion();
  
};
#endif
