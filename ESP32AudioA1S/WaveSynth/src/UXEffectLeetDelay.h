/**   @file  
     @brief  
*/
#pragma once
#ifndef INCLUDED_UXEFFECTLEETDELAY_H
#define INCLUDED_UXEFFECTLEETDELAY_H
#include "stdio.h"
 

/*
desc:Leet Delay 2

/*****************************************************
Copyright (C) 2016 Stige T.
License: http://jsplugins.supermaailma.net/license.php
*****************************************************

EffectName: Leet Delay 2
VendorString: Sonic Anomaly
VendorVersion: 1000
UniqueId: 'LTD2'
*/

  
struct/*class*/ TSPF
{float x; float y1; float y2; float y3; float y4; float oldx; float oldy; float oldy1; float oldy2; float oldy3; float f; float p; float k; float t; float t2; float r; 
  void init(float freq, float res);
  float process (float input);
};

      
struct/*class*/ TAGC
{float env; float tmp; 
float follower(float input, float att, float rel);

};

      
struct/*class*/ TDELAY
{ int sindex; int len; float *buffer;
  TDELAY() : sindex(0),buffer(NULL){}
  void init(int samples,int index);
  float delay(float input);
};


 
//slider1:4<0,23,1{0.25,0.33,0.50,0.66,0.75,1.00,1.25,1.33,1.50,1.66,1.75,2.00,2.25,2.33,2.50,2.66,2.75,3.00,3.33,3.25,3.50,3.66,3.75,4.00}>-Delay [1/4 Notes]
 
struct/*class*/ TXEffectLeetDelay
{int propSl1DelaySyncIndex; // 0..23, indexed
//slider2:500<0,4000,10>-Delay [ms]
 int propSl2DelayMS; // 0..4000
//slider3:0<0,1,1>-Tempo Delay [Off/On]
 bool propSl3TempoDelay;
//slider4:10<0,100,1>-Feedback [%]
  int propSl4Feedback; // 0..100
//slider5:2000<400,4000,10>-Color [Hz]
  int propSl5Color; // 400..4000
//slider6:0.05<0,1,0.01>-Resonance
  float propSl6Resonance; // 0..1
//slider7:1<0,1,0.01>-Color Mix
  float propSl7ColorMix; // 0..1
//slider8:0<0,1,1>-Ping Pong [Off/On]
  bool propSl8PingPong;
//slider10:0<-40,10,1>-Dry [dB]
  int prop10Dry; // -40..10
//slider11:0<-40,10,1>-Wet [dB]
  int prop11Wet; // -40..10
  TSPF spf1; TSPF spf2;
  TDELAY D0; TDELAY D1;
  TAGC agc;

  float FTempo;
  float prm_feedback; float prm_colorMul; float prm_colorMulInv; float prm_fbAgcAtt; float prm_wet; float prm_dry; float prm_tempoMul; float prm_delayMs; float prm_freq; float prm_reso; float prm_delayTime; float delayFromTempo; float fbMod;
  bool prm_pingpong; bool prm_temDelActive;
  int prm_delayMul;

  float fb1; float fb2;
  float s1; float s2;

  void InitDelays();

  void Init();
  void ProcessSliders();
  void Block();
  TXEffectLeetDelay();
  void SetParameter(int index, float value);// value E [0..1]
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);

  void SetTempo(float tempo);
  void SetSampleRate(int samplerate);


};




#endif//INCLUDED_UXEFFECTLEETDELAY_H
//END
