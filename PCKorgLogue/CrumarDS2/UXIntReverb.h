/**   @file  
     @brief  
*/
#pragma once
#ifndef INCLUDED_UXINTREVERB_H
#define INCLUDED_UXINTREVERB_H
 




 
// sliders
/*
slider10:30<0,100,1>-Dampening (%)
slider13:-1<-1,1,0.1>-Stereo Width

slider21:75<0,100,0.2>-L Drive (%)
slider22:9<-16,16,0.2>-L Gain (dB)
slider23:0<0,100,0.2>-M Drive (%)
slider24:0<-16,16,0.2>-M Gain (dB)
slider25:75<0,100,0.2>-H Drive (%)
slider26:12<-16,16,0.2>-H Gain (dB)
slider27:30<20,200,1>-L <> M (Hz)
slider28:8000<720,12000,10>-M <> H (Hz)

slider60:-0.5<-1,1,0.01>-Dry <> Wet
slider63:-3<-24,3,0.5>-Center (dB)
slider64:-3<-24,3,0.5>-Output (dB */

const int a0_len = 1680; // round( srate*35*1.0/1000 );  
const int a1_len = 1056; //round( srate*22*1.0/1000 );
const int a2_len = 400;// round( srate*8.3*1.0/1000 );
const int a3_len = 3168; //round( srate*66*1.0/1000 );
const int a4_len = 1440; // round( srate*30*1.0/1000 );
const int a0r_len = 1680; //round( srate*35*1.0/1000 )+rndcoef;
const int a1r_len = 1006; //round( srate*22*1.0/1000 )-rndcoef;
const int a2r_len = 450; //round( srate*8.3*1.0/1000 )+rndcoef;
const int a3r_len = 3118;//round( srate*66*1.0/1000 )-rndcoef;
const int a4r_len = 1490; //round( srate*30*1.0/1000 )+rndcoef;

/*Total Size = approx. 16000 */


  
struct/*class*/ TReverbGeek
{float dampening; float stereoWidth; float LDrive; float LGain; float MDrive; float MGain; float HDrive; float HGain; float LToM; float MToH; float DryWet; float Center; float Output;
// local data
//?  wet:single;
  float iin; float iout; float c; float g; float a0_in; float a0_out; float a0_g; float a2_out; float a1_in; float a1_out; float a1_g; float a2_ina2_g; float a2_in; float a2_g; float tmpr;
  float a0r_in; float a0r_out; float a2r_out; float a1r_in; float a1r_out; float a2r_in; float a3r_in; float a3r_out; float a4r_out; float a4r_in; float rev1;
  float dry0_l; float dry0_m; float dry0_h; float dry0; float dry1; float low_l; float low_r; float mid_l; float mid_r; float high_l; float high_r;
  float a0LP; float b1HP; float b1LP; float a0HP; float cDenorm;
  float leveldry; float levelwid0; float levelwid1; float levelwet; float levelout; float levelmid;
  float wet0; float wet0_l; float wet0_m; float wet0_h; float mixlg; float mixmg; float mixhg; float mixlg1; float mixmg1; float mixhg1; float gainl; float gainm; float gainh;
  float wet1; float wet1_l; float wet1_m; float wet1_h;
  float dry1_l; float dry1_m; float dry1_h; float out_mid; float out_sid;
  float a3_in; float a3_out; float a3_g; float a4_out; float a4_in; float a4_g;
  float srate;
  float t; float tr; float tmp; float rev0; float tmplLP; float tmprLP; float tmplHP; float tmprHP;

  float mixl; float mixm; float mixh; float freqLP; float xLP; float xHP; float freqHP; float mixl1; float mixm1; float mixh1;
  int a0_pos; int a1_pos; int a2_pos; 
  int a3_pos; int a4_pos; 
  int a0r_pos; int a1r_pos; int a2r_pos;  int a3r_pos;  int a4r_pos; 
  float a0[a0_len];
  float a1[a1_len];
  float a2[a2_len];
  float a3[a3_len];
  float a4[a4_len];
  float a0r[a0r_len];
  float a1r[a1r_len];
  float a2r[a2r_len];
  float a3r[a3r_len];
  float a4r[a4r_len];
  void SliderChanged();
  
private:
  float wet;


public:
  void SetRoomSize(float roomsize);
  void SetFeedback(float feedback);

  virtual void processSample( float & spl0, float & spl1);
	TReverbGeek();
  void setSampleRate(float samplerate);
  void setWet(float wet);
//  void Process(TProcessBuffer Inputs, TProcessBuffer Outputs,   unsigned long SampleFrames);  

};





#endif//INCLUDED_UXINTREVERB_H
//END
