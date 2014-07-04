#include "ConnectionListener.h"
#include "Socket.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

ConnectionListener::ConnectionListener(Server *s, string p) : Task(s)
{
  device = getSocket(p.c_str());
}

// Setup a socket on port, port
int ConnectionListener::getSocket(std::string port)
{
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int sockfd;
  int yes = 1;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use local IP 

  if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      return -1;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return -1;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd,20)) {
    perror("listen fail");
    return -1;
  }

  return sockfd;
}

Command ConnectionListener::run()
{
  cout << "A connection" << endl;
  return QUIT;
}


