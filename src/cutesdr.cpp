// This gloms cutsdr's AD6620 programmer class Cad6620. The little
// main program below calls Cad6620 for each filter index. Each of
// these produces 267 9-byte messages to be sent to the SDR-IQ to 
// program the AD6620 for the filter. These are formatted as text
// and stored in the header file Dsp.h for later use by the radio.
//
#include "ascpmsg.h"
#include "netiobase.h"
#include "ad6620.h"
#include <iostream>
#include <iomanip>
#include "SdrIQ.h"
#include <fstream>
using namespace std;

// Format a message as a c style initializer
ostream &operator<<(ostream &s, message msg)
{
  s << "{";
  for (int n = 0; n < msg.length; n++) {
    int k = msg.data[n];
    k &= 0xFF;
    s << "0x" << uppercase << hex << setfill('0') << setw(2) << k;
    if ( n != msg.length-1 )
      s << ", ";
  }
  s << "}";
  return s;
}

// Make a complete message array using the BWKHZ_???
// enum as the array name. 
#define PCMD(s,x) printCmds(s,#x,Cad6620::x)

void printCmds(ostream &s, string name, int filter)
{
  CAscpTxMsg pAscpMsg;
  Cad6620 ad6620;
  ad6620.m_NCOampDither = false;
  ad6620.m_NCOphsDither = false;
  ad6620.CreateLoad6620Msgs(filter);
  int nm = 0;
  s << "MSG " << "cmd" << name << "[][9] = {" << endl;
  while (ad6620.GetNext6620Msg(pAscpMsg)) {
    s << message(pAscpMsg.Buf8);
    nm++;
    if ( nm != 0x10B )
      s << "," << endl;
  }
  s << "};" << endl;
}
  

// Output each filter program to a header for
// later use. Since these messages can't or 
// aren't altered by cutesdr I didn't see any
// point to splicing in the code that generates
// them.    
int main()
{
  ofstream out("Dsp.h");
  PCMD(out,BWKHZ_5);
  PCMD(out,BWKHZ_10);
  PCMD(out,BWKHZ_25);
  PCMD(out,BWKHZ_50);
  PCMD(out,BWKHZ_100);
  PCMD(out,BWKHZ_150);
  PCMD(out,BWKHZ_190);
  return 0;
}
