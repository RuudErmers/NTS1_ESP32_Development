/**   @file  
     @brief  
*/
#include "UXIntReverb.h"

#include "Math.h"
#include "myOS.h"

 static double/*?*/ const log2db = 8.6858896380650365530225783783321; // 20 / ln(10)
 static double/*?*/ const db2log = 0.11512925464970228420089957273422; // ln(10) / 20
 static double/*?*/ const pi = 3.1415926535;
 static double/*?*/ const halfpi = pi *1.0/ 2;
 static double/*?*/ const halfpiscaled = halfpi * 1.41254;

/**
 * fast approximation of the sine function for range [-pi, pi]
 * @param[in] x float
 * @return result float
 */
float axSin(float x)
{   float result;
  x = x*( 1.2732395447 - 0.4052847345 * ABS( x ) );
  result =0.225 * ( x * ABS( x ) - x ) + x;
return result;
}

void TReverbGeek::SetRoomSize(float roomsize)
{
  DryWet =-1+2*roomsize;
  SliderChanged();
}


void TReverbGeek::processSample( float & spl0, float & spl1)
{ float s0; float s1;

  s0 =spl0;
  s1 =spl1;
  if(  srate == 0 ) return;
// REVERB

// left channel

// ---- AP 1
a0_in = iin;
a0_out = -a0_in*a0_g + a2_out;
a1_in = a0_in + a0_out*a0_g;

// -> nested
  a1_out = -a1_in*a1_g + a1[ a1_pos ];
  a1[ a1_pos ] = a1_in + a1_out*a1_g;
  a1_pos =a1_pos+1;
  if(  ( a1_pos >= a1_len ) ) a1_pos = 0;
  a2_in = a1_out;
  a2_out = -a2_in*a2_g + a2[ a2_pos ];
  a2[ a2_pos ] = a2_in + a2_out*a2_g;
  a2_pos =a2_pos+1;
  if(  ( a2_pos >= a2_len ) ) a2_pos = 0;

  // delay
  a0[ a0_pos ] = a2_out;
  a0_pos =a0_pos+1;
  if(  ( a0_pos >=a0_len ) ) a0_pos = 0;
  a2_out = a0[ a0_pos ];
// <-

// ---- end AP 1

// ---- AP 2
a3_in = a0_out;
a3_out = -a3_in*a3_g + a4_out;
a4_in = a3_in + a3_out*a3_g;

// -> nested
  a4_out = -a4_in*a4_g + a4[ a4_pos ];
  a4[ a4_pos ] = a4_in + a4_out*a4_g;
  a4_pos =a4_pos+1;
  if(  ( a4_pos >= a4_len ) ) a4_pos = 0;

  // delay
  a3[ a3_pos ] = a4_out;
  a3_pos =a3_pos+1;
  if(  ( a3_pos >= a3_len ) ) a3_pos = 0;
  a4_out = a3[ a3_pos ];
// <-

// ---- end AP 2

tmp = a3_out;
t = tmp + c*( t-tmp );
tmp = t;

iout = a0_out*0.5 + a3_out*0.5;

rev0 = iout;


// right channel

iin = spl1 + tmpr*g;

// ---- AP 1
a0r_in = iin;
a0r_out = -a0r_in*a0_g + a2r_out;
a1r_in = a0r_in + a0r_out*a0_g;

// -> nested
  a1r_out = -a1r_in*a1_g + a1r[ a1r_pos ];
  a1r[ a1r_pos ] = a1r_in + a1r_out*a1_g;
  a1r_pos =a1r_pos+1;
  if(  ( a1r_pos >= a1r_len ) ) a1r_pos = 0;

  a2r_in = a1r_out;
  a2r_out = -a2r_in*a2_g + a2r[ a2r_pos ];
  a2r[ a2r_pos ] = a2r_in + a2r_out*a2_g;
  a2r_pos =a2r_pos+1;
  if(  ( a2r_pos >= a2r_len ) ) a2r_pos = 0;

  // delay
  a0r[ a0r_pos ] = a2r_out;
  a0r_pos =a0r_pos+1;
  if(  ( a0r_pos >= a0r_len ) ) a0r_pos = 0;
  a2r_out = a0r[ a0r_pos ];
// <-

// AP 2
a3r_in = a0r_out;
a3r_out = -a3r_in*a3_g + a4r_out;
a4r_in = a3r_in + a3r_out*a3_g;

// -> nested
  a4r_out = -a4r_in*a4_g + a4r[ a4r_pos ];
  a4r[ a4r_pos ] = a4r_in + a4r_out*a4_g;
  a4r_pos =a4r_pos+1;
  if(  ( a4r_pos >= a4r_len ) ) a4r_pos = 0;

  // delay
  a3r[ a3r_pos ] = a4r_out;
  a3r_pos =a3r_pos+1;
  if(  ( a3r_pos >= a3r_len ) ) a3r_pos = 0;
  a4r_out = a3r[ a3r_pos ];
// <-

tmpr = a3r_out;
tr = tmpr + c*( tr-tmpr );
tmpr = tr;

iout = a0r_out*0.5 + a3r_out*0.5;

rev1 = iout;

// BAND EQ

dry0 = spl0 * leveldry + ( rev0 * levelwid0 + rev1 * levelwid1 ) * levelwet;
dry1 = spl1 * leveldry + ( rev0 * levelwid1 + rev1 * levelwid0 ) * levelwet;

tmplLP = a0LP*dry0 - b1LP*tmplLP + cDenorm;
low_l = tmplLP;
tmprLP = a0LP*dry1 - b1LP*tmprLP + cDenorm;
low_r = tmprLP;
tmplHP = a0HP*dry0 - b1HP*tmplHP + cDenorm;
high_l = dry0 - tmplHP;
tmprHP = a0HP*dry1 - b1HP*tmprHP + cDenorm;
high_r = dry1 - tmprHP;

mid_l = dry0 - low_l - high_l;
mid_r = dry1 - low_r - high_r;

wet0_l = mixlg * axSin( low_l * halfpiscaled );
wet0_m = mixmg * axSin( mid_l * halfpiscaled );
wet0_h = mixhg * axSin( high_l * halfpiscaled );
wet0 = ( wet0_l + wet0_m + wet0_h );

dry0_l = low_l * mixlg1;
dry0_m = mid_l * mixmg1;
dry0_h = high_l * mixhg1;
dry0 = ( dry0_l + dry0_m + dry0_h );

wet1_l = mixlg * axSin( low_r * halfpiscaled );
wet1_m = mixmg * axSin( mid_r * halfpiscaled );
wet1_h = mixhg * axSin( high_r * halfpiscaled );
wet1 = ( wet1_l + wet1_m + wet1_h );

dry1_l = low_r * mixlg1;
dry1_m = mid_r * mixmg1;
dry1_h = high_r * mixhg1;
dry1 = ( dry1_l + dry1_m + dry1_h );

spl0 = ( dry0 + wet0 ) * levelout;
spl1 = ( dry1 + wet1 ) * levelout;

out_mid = ( spl0 + spl1 ) * 0.5 * levelmid;
out_sid = ( spl0 - spl1 ) * 0.5;

spl0 = out_mid + out_sid;
spl1 = out_mid - out_sid;

spl0 =wet*s0+( 1-wet )*spl0;
spl1 =wet*s1+( 1-wet )*spl1;
}


void TReverbGeek::SetFeedback(float feedback)
{
  dampening =100-feedback*100;
  SliderChanged();
}

void TReverbGeek::setSampleRate(float samplerate)
{ int rndcoef;

  srate =samplerate;
  if(  srate == 0 ) return;
  // REVERB

  a0_g = 0.3;

  a1_g = 0.4;

  a2_g = 0.6;

  a3_g = 0.1;

  a4_g = 0.4;

  // the lowpass isn't sepcified any further so we use a simple RC filter
  c = exp( -2*3.14*4200*1.0/srate );

}

void TReverbGeek::SliderChanged()
{
  if(  srate == 0 ) return;
  // REVERB

  g = 1-dampening*1.0/100;

  // Register memory usage with system.
  // Unsure if this is correct, so needs testing. Only
  // required if we want to allocate memory for extra effects.
  // heap.min(a4r + a4r_len + 1);

  // BAND EQ

  mixl = LDrive *1.0/ 100;
  mixm = MDrive *1.0/ 100;
  mixh = HDrive *1.0/ 100;

  freqLP = MIN( MIN( LToM, srate ), MToH );//freqHP);
  xLP = exp( -2.0*pi*freqLP*1.0/srate );
  a0LP = 1.0-xLP;
  b1LP = -xLP;

  freqHP = MAX( MIN( MToH, srate ), freqLP );
  xHP = exp( -2.0*pi*freqHP*1.0/srate );

  a0HP = 1.0-xHP;
  b1HP = -xHP;

  mixl1 = 1 - mixl;
  mixm1 = 1 - mixm;
  mixh1 = 1 - mixh;
  gainl = exp( LGain * db2log );
  gainm = exp( MGain * db2log );
  gainh = exp( HGain * db2log );
  mixlg = mixl * gainl;
  mixmg = mixm * gainm;
  mixhg = mixh * gainh;
  mixlg1 = mixl1 * gainl;
  mixmg1 = mixm1 * gainm;
  mixhg1 = mixh1 * gainh;

  // Reverb Stereo Width

  levelwid0 = ( stereoWidth + 1 ) * 0.5;
  levelwid1 = ( 1 - stereoWidth ) * 0.5;

  // Output Settings

  levelwet = ( DryWet + 1 ) * 0.5;
  leveldry = ( 1 - DryWet ) * 0.5;
  levelmid = exp( Center * db2log );
  levelout = exp( Output * db2log );
}

TReverbGeek::TReverbGeek()
{
// REVERB

  a0_pos = 0;
  a1_pos = 0;
  a2_pos = 0;
  a3_pos = 0;
  a4_pos = 0;
  a0r_pos = 0;
  a1r_pos = 0;
  a2r_pos = 0;
  a3r_pos = 0;
  a4r_pos = 0;
  tmp = 0;
  tmpr = 0;
  t = 0;
  tr = 0;
  cDenorm = pow( 10, -30 );

  dampening =60;
  stereoWidth = 0.5;
  LDrive =50;
  LGain =0;
  MDrive =50;
  MGain =0;
  HDrive =50;
  HGain =0;
  LToM =100;
  MToH =4000;
  DryWet =0;
  Center =0;
  Output =0;
  setSampleRate( 44100 );
  wet =0;
  SliderChanged();
}





//END
