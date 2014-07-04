#ifndef __DSPMSG__
#define __DSPMSG__
// Some prepackaged messages. Rather than go through some
// long song and dance to encode radio messages we just
// define them as byte arrays and change the bits that
// require fiddling on the fly. Most of these byte arrays
// never need modification.
#include "SdrIQ.h"
 
extern MSG cmdName[];
extern MSG cmdSerialNumber[];
extern MSG cmdInterfaceVersion[];
extern MSG cmdPIC0Version[];
extern MSG cmdPIC1Version[];
extern MSG cmdStatus[];
extern MSG cmdStop[];
extern MSG cmdFreeRun[];
extern MSG cmdGetN[];
extern MSG cmdGetFreq[];
extern MSG cmdSetFreq[];
extern MSG cmdSetRFGain[];
extern MSG cmdGetRFGain[];
extern MSG cmdSetIFGain[];
extern MSG cmdGetIFGain[];
extern MSG cmdSetSampleRate[];
// Generated using cutesdr's Cad6620 class
extern MSG cmdBWKHZ_5[267][9];
extern MSG cmdBWKHZ_10[267][9];
extern MSG cmdBWKHZ_25[267][9];
extern MSG cmdBWKHZ_50[267][9];
extern MSG cmdBWKHZ_100[267][9];
extern MSG cmdBWKHZ_150[267][9];
extern MSG cmdBWKHZ_190[267][9];

#endif

