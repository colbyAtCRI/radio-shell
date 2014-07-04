#include "Consold.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
typedef unsigned char MSG;

#include "Dsp.h"

using namespace std;

Consold::Consold(Server *s) : Task(s)
{
  device = 0;
  prompt();
  command["freq"] = &Consold::doFrequency;
  command["quit"] = &Consold::doQuit;
  command["stat"] = &Consold::doStatus;
  command["fver"] = &Consold::doFirmwareVersion;
  command["bver"] = &Consold::doBootVersion;
  command["run"]  = &Consold::doRun;
  command["stop"] = &Consold::doStop;
  command["rf-gain"] = &Consold::doRFGain;
  command["if-gain"] = &Consold::doIFGain;
  command["output"] = &Consold::doOutput;
  command["filter"] = &Consold::doBandwidth;
}

// While running the radio can send back 
// a data message before sending a reply 
// to a message. This function processes
// data messages (type 4) until a non-data
// message is recv'd
message Consold::getReply()
{
  message msg;
  while (1) {
    msg = server->recv();
    if ( msg.type == 4 )
      server->broadcast(msg);
    else 
      break;
  }
  return msg;
}

void Consold::prog6620(unsigned char prog[267][9])
{
  for (int n = 0; n < 267; n++) {
    message msg(prog[n]);
    server->send(msg);
    msg = getReply();
  }
}

Command Consold::doBandwidth(Consold::Args &args)
{
  if ( args.size() != 2 ) {
    cout << "Available Bandwidths" << endl;
    cout << "  1 - 5   Khz" << endl;
    cout << "  2 - 10  Khz" << endl;
    cout << "  3 - 25  Khz" << endl;
    cout << "  4 - 50  Khz" << endl;
    cout << "  5 - 100 Khz" << endl;
    cout << "  6 - 150 Khz" << endl;
    cout << "  7 - 190 Khz" << endl;
  }
  else {
    if ( args[1] == "5" )
      prog6620(cmdBWKHZ_5);
    else if ( args[1] == "10" )
      prog6620(cmdBWKHZ_10);
    else if ( args[1] == "25" )
      prog6620(cmdBWKHZ_50);
    else if ( args[1] == "50" )
      prog6620(cmdBWKHZ_50);
    else if ( args[1] == "100" )
      prog6620(cmdBWKHZ_100);
    else if ( args[1] == "150" )
      prog6620(cmdBWKHZ_150);
    else if ( args[1] == "190" )
      prog6620(cmdBWKHZ_190);
    else {
      cout << args[1] << " isn't supported" << endl;
    }
  }
  return CONTINUE;
}

Command Consold::doOutput(Consold::Args &args)
{
  server->outp = fopen(args[1].c_str(),"w");
  return CONTINUE;
}
Command Consold::doIFGain(Consold::Args &args)
{
  int ifg[] = {0,6,12,18,24};
  if ( args.size() == 1 ) {
    cout << "valid settings" << endl;
    for (int k = 0; k < 5; k++)
      cout << "    " << ifg[k] << endl;
    message msg(cmdGetIFGain);
    server->send(msg);
    msg = getReply();
    int g = (char)msg.data[5];
    cout << "IF Gain: " << g << " dB" << endl;
  }
  else {
    message msg(cmdSetIFGain);
    stringstream inp(args[1]);
    int n; 
    inp >> n;
    msg.data[5] = 0xFF & n;
    server->send(msg);
    msg = getReply();
  }
  return CONTINUE;
}

Command Consold::doRFGain(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    message msg(cmdGetRFGain);
    server->send(msg);
    msg = getReply();
    int g = (char)msg.data[5];
    cout << "RF Gain: " << g << " dB" << endl;
  }
  else {
    message msg(cmdSetRFGain);
    stringstream inp(args[1]);
    int n; 
    inp >> n;
    msg.data[5] = 0xFF & n;
    server->send(msg);
    msg = getReply();
  }
  return CONTINUE;
}

Command Consold::doStop(Consold::Args &args)
{
  message msg(cmdStop);
  server->send(msg);
  msg = getReply();
  if ( server->outp ) {
    fclose(server->outp);
    server->outp = NULL;
  }
  return CONTINUE;
}

Command Consold::doRun(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    message msg(cmdFreeRun);
    server->send(msg);
    msg = getReply();
  }
  else {
    message msg(cmdGetN);
    stringstream inp(args[1]);
    int N;
    inp >> N;
    msg.data[7] = N;
    server->send(msg);
    msg = getReply();
  }
  return CONTINUE;
}

Command Consold::doFirmwareVersion(Consold::Args &args)
{
  message msg(cmdPIC1Version);
  server->send(msg);
  msg = getReply();
  unsigned short data = (unsigned short)msg.data[5];
  cout << "Firmware version: " << (double)data/100.0 << endl;
  return CONTINUE;
}

Command Consold::doBootVersion(Consold::Args &args)
{
  message msg(cmdPIC0Version);
  server->send(msg);
  msg = getReply();
  unsigned short data = (unsigned short)msg.data[5];
  cout << "Bootversion: " << (double)data/100.0 << endl;
  return CONTINUE;
}

const
char *Status[] = {"Idle",
	 	  "Busy",
		  "Loading AD6620",
		  "Boot Mode Idle",
		  "Boot Mode Busy",
		  "A/D Overload",
		  "Boot Mode Programming Error"};

Command Consold::doStatus(Consold::Args &args)
{
  message msg(cmdStatus);
  server->send(msg);
  msg = getReply();
  cout << "status: " << Status[msg.data[4]-11] << endl;
  return CONTINUE;
}

Command Consold::doFrequency(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    message msg(cmdGetFreq);
    server->send(msg);
    msg = getReply();
    cout << *(int*)(msg.data+5)/1000000.0 << " MHz" << endl;
  }
  else {
    message msg(cmdSetFreq);
    stringstream num(args[1].c_str());
    double freq;
    num >> freq;
    int ifreq  = (int)1.0E6 * freq;
    unsigned char *data = (unsigned char *)&ifreq;
    for (int k = 0; k < 4; k++)
      msg.data[k+5] = data[k];
    server->send(msg);
    msg = getReply();
  }
  return CONTINUE;
}

Command Consold::doQuit(Consold::Args &args)
{
  return QUIT;
}

vector<string> parse(string line) 
{
  vector<string> wrds;
  int nw = -1;
  string::iterator c;
  enum {
    Start,
    Inword
  } state;

  c = line.begin(); 
  state = Start;
  while (c != line.end()) {
    switch (state) {
    case Start:
      if ( !isspace(*c) ) {
	state = Inword;
	wrds.push_back("");
	nw++;
	wrds[nw] += *c;
      }
      break;
    case Inword:
      if ( !isspace(*c) ) {
	wrds[nw] += *c;
      }
      else {
	state = Start;
      }
      break;
    }
    c++;
  }
  return wrds;
}

Command Consold::run()
{
  Command ret = CONTINUE;
  char c;
  vector<string> wrds;

  line.clear();

  while ( (c = cin.get()) != 10 && c != 13 && c != EOF)
    line += c;

  if ( c == EOF ) {
    cout << endl;
    return QUIT;
  }

  wrds = parse(line);

  if ( !wrds.empty() ) {
    if ( command.find(wrds[0]) != command.end() )
      ret = (this->*command[wrds[0]])(wrds);
    else {
      cout << "what? " << wrds[0] << endl;
    }
  }
  line.clear();
  if ( ret != QUIT ) prompt();
  return ret;
}

void Consold::prompt()
{
  cout << "sdr> ";
  cout.flush();
}

