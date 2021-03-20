#include "myOS.h"
#include <stdlib.h>
#include <math.h>
void OsInit() {}
float myOsRound(float f) { return round(f); } // math
float myOsRandom() 
{
  const int a = 16807;
  const int m = 2147483647;
  static int seed;
  seed = (a * seed) % m;
  return seed / m;
}
    
  


