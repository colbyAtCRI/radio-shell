#ifndef __MYSERVER__
#define __MYSERVER__
#include "SdrIQ.h"
#include "FFT.h"
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
  int              rfswitch;
  char             state;
  int              dwellCount;
  int              dwell;
  int              statePeriod;
  struct addrinfo *info;
  FILE            *outp;
  int              filter;

  bool             iqBaseline;
  double           circularity;

  FFT              fft;

  Server(std::string name,std::string sname);

  bool    getDataSocket(std::string port);
  void    add(pTask tsk);
  Command run();
  int     loop();
  void    setSwitchState(char);
  void    changeState();
  message getReply();
  void    broadcast(message msg);
  bool    send(message msg);
  message recv();

  void    prog6620(MSG msg[267][9]);
  void    setFrequency(int freq);
  int     getFrequency();
  void    setIFGain(int g);
  int     getIFGain();
  void    setRFGain(int gain);
  int     getRFGain();
  void    setFilterIndex(int n);
  void    stop();
  void    runFree();
  void    runN(int n);
  int     status();


  std::string  getName();

  double  getFirmwareVersion();
  
};
#endif
