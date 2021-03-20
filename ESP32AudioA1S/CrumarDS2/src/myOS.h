#ifndef __OS_H
#define __OS_H

#include <stdio.h>
#include "Arduino.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(x) ((x)<0 ? -(x) : (x))
#define RANDOM myOsRandom
#define ROUND  myOsRound
#define DEBUG Serial.printf
typedef unsigned char byte;
extern void OsInit();
extern float myOsRound(float f);
extern float myOsRandom(); 

//#define SampleRate 44100

#endif