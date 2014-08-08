#include "Consold.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "Dsp.h"

using namespace std;

Consold::Consold(Server *s) : Task(s)
{
  device = 0;
  prompt();
  command["freq"]        = &Consold::doFrequency;
  command["pan"]         = &Consold::doPan;
  command["quit"]        = &Consold::doQuit;
  command["stat"]        = &Consold::doStatus;
  command["fver"]        = &Consold::doFirmwareVersion;
  command["bver"]        = &Consold::doBootVersion;
  command["run"]         = &Consold::doRun;
  command["stop"]        = &Consold::doStop;
  command["rf-gain"]     = &Consold::doRFGain;
  command["if-gain"]     = &Consold::doIFGain;
  command["output"]      = &Consold::doOutput;
  command["filter"]      = &Consold::doBandwidth;
  command["baseline"]    = &Consold::doBaseline;
  command["circularity"] = &Consold::doCircularity;
  command["switch"]      = &Consold::doSwitch;
}

Command Consold::doSwitch(Consold::Args &args)
{
  if (args.size() == 1)
    cout << "switch state: " << server->state << endl;
  else if ( args[1] == "1" || args[1] == "2" ) 
    server->setSwitchState(args[1][0]);
  else {
    cout << "Valid switch states 1 or 2" << endl;
    cout << "switch state: " << server->state << endl; 
  }
  return CONTINUE;
}

Command Consold::doCircularity(Consold::Args &args)
{
   if (args.size() == 1) 
      cout << "circularity: " << server->circularity << endl;
   else {
      stringstream inp(args[1]);
      inp >> server->circularity;
   }
   return CONTINUE;
}

Command Consold::doBaseline(Consold::Args &args)
{
  if ( args.size()==1 )
    cout << "baseline " << ((server->iqBaseline)?"on":"off") << endl;
  else if ( args[1] == "on" )
    server->iqBaseline = true;
  else if ( args[1] == "off" )
    server->iqBaseline = false;
  else 
    cout << "what? baseline on|off" << endl;
  return CONTINUE;
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
    cout << "filter: " << server->filter << " kHz" << endl;
  }
  else {
    stringstream inp(args[1]);
    int fi;
    inp >> fi;
    server->setFilterIndex(fi);
  }
  return CONTINUE;
}

Command Consold::doOutput(Consold::Args &args)
{
  server->outp = fopen(args[1].c_str(),"w");
  if ( args.size() == 2 ) 
    server->fft.nSets = 1000;
  else {
    stringstream num(args[2]);
    num >> server->fft.nSets;
  }
    
  return CONTINUE;
}

Command Consold::doIFGain(Consold::Args &args)
{
  int ifg[] = {0,6,12,18,24};
  if ( args.size() == 1 ) {
    cout << "valid settings" << endl;
    for (int k = 0; k < 5; k++)
      cout << "    " << ifg[k] << endl;
    cout << "IF Gain: " << server->getIFGain() << " dB" << endl;
  }
  else {
    stringstream inp(args[1]);
    int n; 
    inp >> n;
    server->setIFGain(n);
  }
  return CONTINUE;
}

Command Consold::doRFGain(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    cout << "RF Gain: " << server->getRFGain() << " dB" << endl;
  }
  else {
    stringstream inp(args[1]);
    int gain; 
    inp >> gain;
    server->setRFGain(gain);
  }
  return CONTINUE;
}

Command Consold::doStop(Consold::Args &args)
{
  server->stop();
  return CONTINUE;
}

Command Consold::doRun(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    server->runFree();
  }
  else {
    stringstream inp(args[1]);
    int N;
    inp >> N;
    server->runN(N);
  }
  server->fft.clear();
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
	 	  "Running",
		  "Loading AD6620",
		  "Boot Mode Idle",
		  "Boot Mode Busy",
		  "A/D Overload",
		  "Boot Mode Programming Error"};

Command Consold::doStatus(Consold::Args &args)
{
  int n = server->status();
  if ( n > -1 && n < 7 )
    cout << "status: " << Status[n] << endl;
  return CONTINUE;
}

Command Consold::doFrequency(Consold::Args &args)
{
  if ( args.size() == 1 ) {
    int ifreq = server->getFrequency();
    cout << ifreq/1000000.0 << " MHz" << endl;
  }
  else {
    int ifreq;
    stringstream num(args[1]);
    double freq;
    num >> freq;
    if ( args[1][0] == '+' || args[1][0] == '-') {
       ifreq = server->getFrequency() + (int)1.0E6*freq;
    }
    else {
       ifreq  = (int)1.0E6 * freq;
    }
    server->setFrequency(ifreq);
  }
  return CONTINUE;
}

Command Consold::doPan(Consold::Args &args)
{
  switch (args.size()) {
  case 1: {
    cout << "pan to-freq" << endl;
    cout << "pan to-freq step" << endl;
    cout << "pan from-freq to-freq step" << endl;
  }
    break;
  case 2: {
    stringstream tof(args[1]);
    double xf;
    int fto, f;
    int fat = server->getFrequency();
    int step = 30;
    tof >> xf;
    fto = 1000000 * xf;
    f = fat;
    if ( fto < fat ) {
      while (f-step > fto) {
	f -= step;
	server->setFrequency(f);
      }
      server->setFrequency(fto);
    }
    else {
      while (f+step < fto) {
	f += step;
	server->setFrequency(f);
      }
      server->setFrequency(fto);
    }
  }
    break;  
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

