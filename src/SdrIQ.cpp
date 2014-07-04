#include "SdrIQ.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

using namespace std;

// Use termios to setup an open file in raw mode 
int openUSBdevice(string dev)
{
  int            fd;
  struct termios trm;
  
  fd = open(dev.c_str(),O_RDWR);
  if ( fd < 0 ) 
    return fd;

  if ( tcgetattr(fd,&trm) != 0 )
    return false;  

  trm.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  trm.c_oflag &= ~OPOST;
  trm.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  trm.c_cflag &= ~(CSIZE | PARENB);
  trm.c_cflag |= CS8;
  cfsetspeed(&trm,B38400);
  trm.c_cc[VTIME] = 0;
  
  if ( tcsetattr(fd,TCSANOW,&trm) != 0 )
    return -1;

  return fd;
}

message recvMessage(int fd)
{
  static unsigned char buffer[8194];
  message msg;
  msg.type   = -1;
  msg.length = 0;
  msg.data   = NULL;
  int nb = 0;
  int nr;

  while ( (nr = read(fd,(void*)(buffer+nb),1)) == 1 ) {
    nb += nr;
    if ( nb == 2 ) {
      msg.length = buffer[0]+(buffer[1]&0x1F)*256;
      msg.type = (buffer[1] >> 5)&0x07;
      if ( msg.length == 0 )
	msg.length = 8194;
    }
    if ( nb == msg.length || nb == 8194 )
      break;
  }
  if ( nb == msg.length )
    msg.data = buffer;
  return msg;
}

bool sendMessage(int fd, message msg)
{
  int nw = write(fd,msg.data,msg.length);
  if ( nw != msg.length )
    return false;
  return true;
}


