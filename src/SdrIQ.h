#ifndef __SDRIQ__
#define __SDRIQ__
#include <string>

typedef unsigned char uchar;
typedef unsigned char MSG;

// All messages are serial to and from the radio so we only need one message
// data buffer. The `data' pointer points to a static memory location.

struct message 
{
  int            type;
  int            length;
  unsigned char *data;
  message() {}
  message(unsigned char *buff) {
    data   = buff;
    length = data[0]+(data[1]&0x1F)*256;
    type   = (data[1] >> 5)&0x07;
  }
};

message recvMessage(int fd);

bool sendMessage(int fd, message msg);

int openUSBdevice(std::string name);

#endif
