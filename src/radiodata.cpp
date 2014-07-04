/*
** Take datagrams from sdr-iq server
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "3490"

#define MAXBUFLEN 8194

void *get_in_addr(struct sockaddr_in *sa)
{
  if (sa->sin_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  socklen_t addr_len;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0 ) {
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }
    
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }

  if (p == NULL ) {
    fprintf(stderr, "listener: fail to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);

  printf("waiting for data\n");

  addr_len = sizeof their_addr;
  while (1) {
    numbytes = recvfrom(sockfd, buf, MAXBUFLEN,0,(struct sockaddr*)&their_addr, &addr_len);
     if (numbytes == -1) {
        perror("recvfrom");
        exit(1);
     }
     printf("listener: got %d bytes\n",numbytes);
  }

  close(sockfd);
  return 0;
}
