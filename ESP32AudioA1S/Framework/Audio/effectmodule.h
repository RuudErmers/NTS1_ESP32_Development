/*!
 *  @file       efectmodule.h
 *  Project     Blackstomp Arduino Library
 *  @brief      Blackstomp Library for the Arduino
 *  @author     Hasan Murod
 *  @date       19/11/2020
 *  @license    MIT - Copyright (c) 2020 Hasan Murod
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
  
#ifndef EFFECTMODULE_H_
#define EFFECTMODULE_H_

#include <Arduino.h>

typedef enum
{
  IM_LR,    //input mode = line-in L, line-in R
  IM_LMIC   //input mode = Line-in L, Microphone
} 
INPUT_MODE;

class effectModule
{
  public:
  String name;  //the name of your effect pedal
  INPUT_MODE inputMode; 

  //base class constructor, do basic initialization, don't write a constructor in your descendant class
  effectModule();
  //base desctructor, do basic deallocation, don't write a desttuctor in your descendant class
  ~effectModule();

  //you have to write all initialization in the init() function of the  descendant class to set up
  //the name, inMode, and controls, memory allocation, and any other initialization
  virtual void init(){};

  //you have to write all deallocatoin in the deInit() function of the  descendant class for any deallocation
  virtual void deInit(){};

  //you have to always overload with your own process() function in your descendant class
  virtual void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)=0;

};

#endif
