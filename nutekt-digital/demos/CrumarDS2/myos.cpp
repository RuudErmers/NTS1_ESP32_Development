#include "myOS.h"
#include <stdlib.h>
#include <math.h>
void OsInit() {}
float myOsRound(float f) { return round(f); } // math
float myOsRandom()  { return rand();}  // stdlib


/*
#ifndef __MYOS__H
#define __MYOS__H

#include "Arduino.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(x) ((x)<0 ? -(x) : (x))
#define random rand()
#define DEBUG Serial.printf
#define Round round

#endif
*/