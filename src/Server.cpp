#include "Server.h"
#include "Dsp.h"
#include "Socket.h"
#include "Consold.h"
#include "ConnectionListener.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace std;

Task::Task()
{
  server = NULL;
  device = -1;
}

Task::Task(Server *s) : server(s)
{
  device = -1;
}

// Need to open the radio here.
Server::Server(string name,string switchname) : Task(this), fft(2048)
{
  iqBaseline = false;
  circularity = 1.0;
  ndata = 0;
  dwell = 0;
  dwellCount = 10;
  statePeriod = 100;
  outp = NULL;
  device = openUSBdevice(name);
  if ( device < 0 ) { 
    cout << "Unable to open: " << name << endl;
    exit(1);
  }
  else {
    setFilterIndex(100);
    setFrequency(10001000);
    setIFGain(24);
    setRFGain(0);
  }
  if (!getDataSocket("3490")) {
    cout << "can't open datagram socket" << endl;
    exit(1);
  }
  rfswitch = openUSBdevice(switchname);
  if ( rfswitch < 0 ) {
    cout << "Unable to open Arduino" << endl;
    state = 'x';
  } else {
    setSwitchState('1');
  }
  add(this);
  add(new Consold(this));
  //add(new ConnectionListener(this,"3490"));
  //add(new ConnectionListener(this,"3491"));
}

void Server::runFree()
{
  message msg(cmdFreeRun);
  server->send(msg);
  msg = getReply();
}

void Server::runN(int ns)
{
  message msg(cmdGetN);
  msg.data[7] = ns;
  server->send(msg);
  msg = getReply();
}

int Server::status()
{
  message msg(cmdStatus);
  server->send(msg);
  msg = getReply();
  return msg.data[4]-11;
}

void Server::stop()
{
  message msg(cmdStop);
  send(msg);
  msg = getReply();
}

void Server::setSwitchState(char c)
{
  if ( rfswitch > 0 && (c == '1' || c == '2')) {
    if ( write(rfswitch,&c,1) != 1 ) { 
      cerr << "Problem setting switch" << endl;
    }
    else {
      state = c;
      dwell = dwellCount;
    }
  }
}

void Server::changeState()
{
  if ( state == '1' ) {
    setSwitchState('2');
  }
  else {
    setSwitchState('1');
  }
  dwell = dwellCount;
}

// Most radio commands will get an ack or
// data back from the radio. However, this
// reply is often after the current IQ data 
// block. This function waits for the ack
// or reply while processing the current IQ 
// data.
message Server::getReply()
{
  message msg;
  while (1) {
    msg = recv();
    if ( msg.type == 4 )
      broadcast(msg);
    else 
      break;
  }
  return msg;
}

void Server::prog6620(MSG prog[267][9])
{
  for (int n = 0; n < 267; n++) {
    message msg(prog[n]);
    server->send(msg);
    msg = getReply();
  }
}

void Server::setFrequency(int freq)
{
  message msg(cmdSetFreq);
  MSG *data = (MSG *)&freq;
  for (int k = 0; k < 4; k++)
    msg.data[k+5] = data[k];
  send(msg);
  msg = getReply();
}

int Server::getFrequency()
{
  message msg(cmdGetFreq);
  send(msg);
  msg = getReply();
  return *(int*)(msg.data+5);
}

void Server::setIFGain(int gain)
{
  message msg(cmdSetIFGain);
  msg.data[5] = 0xFF & gain;
  send(msg);
  msg = getReply();
}

int Server::getIFGain()
{
  message msg(cmdGetIFGain);
  send(msg);
  msg = getReply();
  int g = (char)msg.data[5];
  return g;
}

void Server::setRFGain(int gain)
{
  message msg(cmdSetRFGain);
  msg.data[5] = 0xFF & gain;
  send(msg);
  msg = getReply();
}

int Server::getRFGain()
{
  message msg(cmdGetRFGain);
  send(msg);
  msg = getReply();
  int gain = (char)msg.data[5];
  return gain;
}

void Server::setFilterIndex(int fn)
{
  int ifg = getIFGain();
  switch (fn) {
  case 5:
    filter = fn;
    prog6620(cmdBWKHZ_5);
    break;
  case 10:
    filter = fn;
    prog6620(cmdBWKHZ_10);
    break;
  case 25:
    filter = fn;
    prog6620(cmdBWKHZ_50);
    break;
  case 50:
    filter = fn;
    prog6620(cmdBWKHZ_50);
    break;
  case 100:
    filter = fn;
    prog6620(cmdBWKHZ_100);
    break;
  case 150:
    filter = fn;
    prog6620(cmdBWKHZ_150);
    break;
  case 190:
    filter = fn;
    prog6620(cmdBWKHZ_190);
    break;
  }
  setIFGain(ifg);
}

// Setup a datagram socket on port, port
bool Server::getDataSocket(std::string port)
{
  struct addrinfo hints, *servinfo;
  int rv;
  int sockfd;
  int yes = 1;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo("localhost", port.c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return false;
  }

  // loop through all the results and bind to the first we can
  for(info = servinfo; info != NULL; info = info->ai_next) {
    if ((data_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    break;
  }

  if (info == NULL) {
    fprintf(stderr, "server: failed to get data socket\n");
    return false;
  }
  return data_socket > 0;
}

void Server::add(pTask task)
{
  push_back(task);
}

int Server::loop()
{
  bool     running = true;
  fd_set   rds;
  iterator task;
  int      maxdev = -1;
  int      ready;
  Command  cmd;

  // Main server loop
  while (running) {
    
    FD_ZERO(&rds); 

    // Each task has its own device to read from. 
    // If the device number is negative erase the 
    // task;
    for (task = begin(); task != end(); task++) {
      if ( (*task)->device < 0 ) {
	delete *task;
	erase(task);
	continue;
      }
      maxdev = max(maxdev,(*task)->device);
      FD_SET((*task)->device,&rds);
    }

    if ( empty() || maxdev < 0 ) {
      running = false;
      continue;
    }

    ready = select(maxdev+1,&rds,NULL,NULL,NULL);

    //cout << "Got one of: " << size() << endl;

    // Run the task that has input ready for reading
    for (task = begin(); task != end(); task++) {
      if ( FD_ISSET((*task)->device,&rds) ) {
	cmd = (*task)->run();
	break;
      }
    }

    // Process server command passed back
    // from task.
    switch (cmd) {
    case QUIT:
      running = false;
      break;
    case CONTINUE:
      break;
    }
  }
  
  return -1;
};

/*
struct Data
{
  short int I;
  short int Q;
};
*/

void Server::broadcast(message msg)
{
  //Data *data;
  int nb = 0;
  Data *data = (Data*)(msg.data+2);
  double IA = 0.0;
  double QA = 0.0;
  if ( iqBaseline ) {
     for (int k = 0; k < 2048; k++) {
        IA += data[k].I;
        QA += data[k].Q;
     }
     IA /= 2048.0;
     QA /= 2048.0;
     for (int k = 0; k < 2048; k++) {
        data[k].I -= IA;
        data[k].Q -= QA;
     } 
  }

  if ( circularity != 1.0 ) {
     for (int k = 0; k < 2048; k++) 
        data[k].Q *= circularity;
  }

  if (outp != NULL && dwell == 0) {
    data = (Data*)(msg.data+2);
    if (fft.addData(state,data,2048)) {
      for (int n = 0; n < 2048; n++)
	fprintf(outp,"%f %f\n",fft.spectra1[n],fft.spectra2[n]);
      cout << "data done" << endl; 
      fclose(outp);
      outp = NULL;
      stop();
    }
  }

  while (1) {
    nb += sendto(data_socket,
		 msg.data+nb,
		 msg.length-nb,
		 0,
		 info->ai_addr,
		 info->ai_addrlen);
    if ( nb >= msg.length )
      break;
  }

}
// This is called when the sdrIQ says something
// which is not a response to a direct user 
// command
Command Server::run()
{
  message msg;
  msg = recv();
  if ( msg.type == 4 ) {
    ndata++;
    while (dwell > 0)
      dwell--;
    broadcast(msg);
    if ( ndata % 100 == 0 )
      changeState();
  }
  if ( msg.type == 1 ) {
    ndata = 0;
    if ( outp ) {
      fclose(outp);
      outp = NULL;
    }
  }
  return CONTINUE;
}

// Every task has a pointer to the server
// so it can send messages and receive Acks
bool Server::send(message msg) 
{
  if ( device > 0 ) 
    return sendMessage(device,msg);
  else 
    return false;
}

message Server::recv()
{
  return recvMessage(device);
}

int
main()
{
  Server  server("/dev/ttyUSB0","/dev/ttyACM0");
  server.loop();
  return 0;
}
