#include "Server.h"
#include "Socket.h"
#include "Consold.h"
#include "ConnectionListener.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <ncurses.h>

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
Server::Server(string name) : Task(this)
{
  ndata = 0;
  outp = NULL;
  device = openUSBdevice(name);
  if ( device < 0 ) { 
    cout << "Unable to open: " << name << endl;
    exit(1);
  }
  if (!getDataSocket("3490")) {
    cout << "can't open datagram socket" << endl;
    exit(1);
  }
  add(this);
  add(new Consold(this));
  //add(new ConnectionListener(this,"3490"));
  //add(new ConnectionListener(this,"3491"));
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

struct Data
{
  short int I;
  short int Q;
};

void Server::broadcast(message msg)
{
  Data *data;
  int nb = 0;
  while (1) {
    if ( outp == NULL ) {
    nb += sendto(data_socket,
		 msg.data+nb,
		 msg.length-nb,
		 0,
		 info->ai_addr,
		 info->ai_addrlen);
    if ( nb >= msg.length )
      break;
    }
    else {
      data = (Data*)(msg.data+2);
      for (int n = 0; n < 2048; n++)
	fprintf(outp,"%d %d\n",data[n].I,data[n].Q);
      break;
    }
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
    broadcast(msg);
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
  Server  server("/dev/ttyUSB0");
  server.loop();
  return 0;
}
