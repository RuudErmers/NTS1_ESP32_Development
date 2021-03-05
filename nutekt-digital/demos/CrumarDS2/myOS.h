#ifndef __OS_H
#define __OS_H

//#include <stdlib.h>
#include <stdio.h>
//#include <math.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(x) ((x)<0 ? -(x) : (x))
#define RANDOM myOsRandom
#define ROUND  myOsRound
#define DEBUG(a) 
typedef unsigned char byte;
extern void OsInit();
extern float myOsRound(float f);
extern float myOsRandom(); 
extern void OSC_SetParameter(uint8_t cc, uint8_t value);
extern void OSC_PARAM_NEW(uint16_t index, uint16_t value);




//#define SampleRate 44100

#endif