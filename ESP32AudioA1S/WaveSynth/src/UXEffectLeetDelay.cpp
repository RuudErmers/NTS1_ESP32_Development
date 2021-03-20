/**   @file  
     @brief  
*/
#include "UXEffectLeetDelay.h"

#include "Myos.h"

 float hpfreq; float att; float rel; float fadeOut; float env_inertia; float harmonics; float delayDrift; float weight;
 int srate; int maxDelay;
 static float const mulTbl[24] = { 0.25, 0.33, 0.50, 0.66, 0.75, 1.00, 1.25, 1.33, 1.50, 1.66, 1.75, 2.00, 2.25, 2.33, 2.50, 2.66, 2.75, 3.00, 3.33, 3.25, 3.50, 3.66, 3.75, 4.00 };

TXEffectLeetDelay::TXEffectLeetDelay() :   prm_feedback(0),prm_colorMul(0), prm_colorMulInv(0),prm_fbAgcAtt(0), prm_wet(0), prm_dry(0), prm_tempoMul(0), prm_delayMs(0),  prm_freq(0),  prm_reso(0),prm_delayTime(0),delayFromTempo(0),fbMod(0),
  prm_pingpong(false),prm_temDelActive(false),prm_delayMul(0),fb1(0),fb2(0),s1(0),s2(0)

{
  srate =44100;
  maxDelay = srate * 6; // why 6 instead of 4??
//slider1:4<0,23,1{0.25,0.33,0.50,0.66,0.75,1.00,1.25,1.33,1.50,1.66,1.75,2.00,2.25,2.33,2.50,2.66,2.75,3.00,3.33,3.25,3.50,3.66,3.75,4.00}>-Delay [1/4 Notes]
 propSl1DelaySyncIndex=3;
//slider2:500<0,4000,10>-Delay [ms]
 propSl2DelayMS =240;
//slider3:0<0,1,1>-Tempo Delay [Off/On]
 propSl3TempoDelay =true;
//slider4:10<0,100,1>-Feedback [%]
  propSl4Feedback =50;
//slider5:2000<400,4000,10>-Color [Hz]
  propSl5Color =2000;
//slider6:0.05<0,1,0.01>-Resonance
  propSl6Resonance =0.05;
//slider7:1<0,1,0.01>-Color Mix
  propSl7ColorMix =0.3;
//slider8:0<0,1,1>-Ping Pong [Off/On]
  propSl8PingPong =false;
//slider10:0<-40,10,1>-Dry [dB]
  prop10Dry =-5;
//slider11:0<-40,10,1>-Wet [dB]
  prop11Wet =-5;
  FTempo =120;
  Init();
  ProcessSliders();
  InitDelays();  
}

#define pi 3.1415
void TXEffectLeetDelay::Init()
{
  hpfreq = 1-exp( -2*pi*150*1.0/srate );
  att = 0;
  rel = exp( -1*1.0/( srate*0.05 ) );
  fadeOut = exp( -1*1.0/( srate*3 ) );
  env_inertia = exp( -1*1.0/( srate*0.02 ) );
  harmonics = 2;
  delayDrift = srate * 0.00023;
  weight = 1-exp( -1*1.0/( 300 *1.0/ 1000 * srate ) );
}

void TXEffectLeetDelay::SetSampleRate(int samplerate)
{
  srate =samplerate;
}


void TXEffectLeetDelay::SetTempo(float tempo)
{
  FTempo =tempo;
}


float Interpolate(float A, float B, float x)
{   float result;
  result =A + ( ( B-A )*x );return result;
}

void TDELAY::init( int samples,int index)
//instance (len,sloop,splay)
{
   len = (samples > maxDelay) ? maxDelay : samples;
    if(buffer ==NULL) 
    {
      buffer = (float*)ps_malloc(maxDelay*sizeof(float));
      for (int i=0;i<maxDelay;i++) buffer[i]=0;
    }  
}

float TDELAY::delay(float input)
//instance(sindex,splay,sloop,len)
{   float result;
  {
    buffer[ sindex ] = input;
    sindex =sindex+1;
    if(  sindex >= len ) sindex = 0;
    result = buffer[ sindex ];
  }
return result;
}

void TSPF::init(float freq, float res)
//instance(f,p,k,t,t2,r) (
{
  /*?*//* WITH  spf */
  {
    f = ( freq+freq ) *1.0/ srate;
    p = f * ( 1.8-0.8*f );
    k = p + p -1.0;
    t = ( 1.0-p ) * 1.386249;
    t2 = 12.0 + t * t;
    r = res * ( t2+6.0*t ) *1.0/ ( t2-6.0*t );
  }
}

float TSPF::process(float input)
//instance(x,y1,y2,y3,y4,oldx,oldy,oldy1,oldy2,oldy3,f,p,k,t,t2,r)
{   float result;
  /*?*//* WITH  spf */
  {
    x = input - r * y4;
    y1 = x * p + oldx * p - k * y1;
    y2 = y1 * p + oldy1 * p - k * y2;
    y3 = y2 * p + oldy2 * p - k * y3;
    y4 = y3 * p + oldy3 * p - k * y4;
    y4 = y4 - ( ( y4*y4*y4 )*1.0/6.0 );
    oldx = x;
    oldy1 = y1;
    oldy2 = y2;
    oldy3 = y3;
    result =y4;
  }
return result;
}

float TAGC::follower(float input, float att, float rel)
//instance (env,tmp)
{   float result;
  /*?*//* WITH  agc */
  {
    if(  ( input >= tmp ) )
     tmp =  input;else
     tmp = input + env_inertia * ( tmp-input );
    if(  tmp > env )
      env = att * ( env - tmp ) + tmp;else
      env = rel * ( env - tmp ) + tmp;
    result =env;
  }
return result;
}

void TXEffectLeetDelay::InitDelays()
{
  D0.init(trunc(MAX(prm_delayTime-delayDrift,0.0)),0);
  D1.init(trunc(MAX(prm_delayTime+delayDrift,delayDrift*2)),1);
}

void TXEffectLeetDelay::ProcessSliders()
{ float attCoeff;

  prm_feedback = propSl4Feedback *1.0/ 100;
  prm_delayMul = propSl1DelaySyncIndex;
  prm_tempoMul = mulTbl[ prm_delayMul ];
  prm_delayMs = srate*1.0/( 1*1.0/( propSl2DelayMS*1.0/1000 ) );
  prm_freq = propSl5Color;
  prm_reso = propSl6Resonance;
  prm_colorMul = propSl7ColorMix;
  prm_colorMulInv = ( 1-prm_colorMul );
  prm_pingpong = propSl8PingPong;

  prm_dry = pow( 10, prop10Dry*1.0/20 );
  prm_wet = pow( 10, prop11Wet*1.0/20 );
  prm_temDelActive = propSl3TempoDelay;
  attCoeff = pow( MAX( prm_feedback-0.5, 0 )*2, 0.01 );
  prm_fbAgcAtt = exp( -1*1.0/( srate*Interpolate( 0.05, 0, attCoeff ) ) );

  spf1.init(prm_freq*1.1, prm_reso );
  spf2.init(prm_freq*0.9, prm_reso );
  hpfreq = 1-exp( -2*pi*150*( prm_colorMul+1 )*1.0/srate );
  s1=s2=0;

  if(  prm_temDelActive )
  {
    prm_delayTime = prm_delayMs;
    InitDelays();
  }

  delayFromTempo = 0;
}

void TXEffectLeetDelay::Block()
{
  if(  prm_temDelActive && ( delayFromTempo==0 ) )
  {
    delayFromTempo = srate *1.0/ ( FTempo *1.0/ 60 );
    prm_delayTime = delayFromTempo*prm_tempoMul;
    prm_delayTime = delayFromTempo*prm_tempoMul;
    InitDelays();
  }
}

/*$POINTERMATH ON*/
void TXEffectLeetDelay::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)

{ float norm;
    float in1; float in2;
    float env;
    int sample;
  Block();
  for( sample = 0 ; sample < sampleCount ; ++sample )
  {
    norm = ( myOsRandom() * 0.0000000001 );

    if(  !prm_pingpong )
    {
      in1 = inLeft[ sample ] + ( fb1 * prm_feedback ) + norm;
      in2 = inRight[ sample ]+ ( fb2 * prm_feedback ) + norm;
      // Delay 
      fb1 = D0.delay(in1 * 0.962 );
      fb2 = D1.delay(in2 );
    }
    else
    {
      in1 = inLeft[ sample ] + ( fb2 * prm_feedback ) + norm;
      in2 = inRight[ sample ] + ( fb1 * prm_feedback ) + norm;

      // Delay 
      fb2 = D1.delay(in2 * 0.92 );
      fb1 = D0.delay(in1 * 0.97 );
    }
    

    // HPF Filter 
    s1 = s1 + hpfreq *  ( fb1 - s1 );
    fb1 = fb1 - s1;
    s2 = s2 + hpfreq *  ( fb2 - s2 );
    fb2 = fb2 - s2;  
    // Color 
  
   // fb1 = spf1.process(fb1 ) * prm_colorMul +  ( fb1 * prm_colorMulInv );
   // fb2 = spf2.process(fb2 ) * prm_colorMul +  ( fb2 * prm_colorMulInv ); 
  
    // Feedback AGC 
    env = agc.follower( abs( fb1 )+abs( fb2 )*0.5 , prm_fbAgcAtt, rel );
    if(  env > 0.001 )
      fbMod = 0.891*1.0/( env+0.5 );else
      fbMod = fbMod*fadeOut;
    fb1 = fb1*fbMod;
    fb2 = fb2*fbMod; 

    // Sum 
    outLeft[ sample ] = ( -fb1 * prm_wet ) + ( inLeft[ sample ] * prm_dry );
    outRight[ sample ] = ( fb2 * prm_wet ) +  ( inRight[ sample ] * prm_dry );
  }
}


void TXEffectLeetDelay::SetParameter(int index, float value)// value E [0..1]
{
   switch( index ) {
  //slider1:4<0,23,1{0.25,0.33,0.50,0.66,0.75,1.00,1.25,1.33,1.50,1.66,1.75,2.00,2.25,2.33,2.50,2.66,2.75,3.00,3.33,3.25,3.50,3.66,3.75,4.00}>-Delay [1/4 Notes]
    case 0:  propSl1DelaySyncIndex =round( value*23 ); // 0..23, indexed
//slider2:500<0,4000,10>-Delay [ms]
    case 1: propSl2DelayMS =round( 4000*value );
//slider3:0<0,1,1>-Tempo Delay [Off/On]
    case 2: propSl3TempoDelay =value>=0.5;
//slider4:10<0,100,1>-Feedback [%]
    case 3: propSl4Feedback =round( 100*value );
//slider5:2000<400,4000,10>-Color [Hz]
    case 4: propSl5Color =round( 400+3600*value );
//slider6:0.05<0,1,0.01>-Resonance
    case 5: propSl6Resonance =value;
//slider7:1<0,1,0.01>-Color Mix
    case 6: propSl7ColorMix =value;
//slider8:0<0,1,1>-Ping Pong [Off/On]
    case 7: propSl8PingPong =value>=0.5;
//slider10:0<-40,10,1>-Dry [dB]
    case 8: prop10Dry =round( -40+50*value );
//slider11:0<-40,10,1>-Wet [dB]
    case 9: prop11Wet =round( -40+50*value );
  }
  ProcessSliders();
}
 
#define EFF 
#ifdef EFF 

TXEffectLeetDelay *myDelay;

void processEffects(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{
  myDelay->process(inLeft, inRight, outLeft, outRight, sampleCount);
}		

void initEffects()
{
  myDelay = new TXEffectLeetDelay();
}	
#endif
//END
