/*
BSD 3-Clause License

Copyright (c) 2018, KORG INC.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/

/*
* File: CrumarDS2.cpp
*
* Naive square oscillator test
*
*/

#include "userosc.h"
#include "myos.h"

#include "UXOscillator.h"
#include "XSynthBlockADSR.h"
#include "XMoogFilter.h"
#include "XSynthConstants.h"

// Two Oscillators, Two LFOs..

struct TOsc {
	TWaveShape Wave;
	float Level;
	float ModDepth;
	int ModTarget;
	int Foot;
	int Semi;
//  TOsc() : Wave(0),Level(0),ModDepth(0),ModTarget(0),Foot(0),Semi(0) {}
};
struct TVCF {
	float Attack;
	float Decay;
	float Release;
	float Sustain;
	float ModDepth;
	int ModTarget;
	float Cutoff; 
    float Resonance;
	float Level;
};
struct TVCA {
	float Attack;
	float Decay;
	float Release;
	float Sustain;
	float ModDepth;
	int ModTarget;
	float Level;
};
struct TLFO {
	TWaveShape Wave;
	float speed;
	bool delay;
	float value; // current lfo value
};
struct TPWM {
	float ModDepth;
	int ModTarget;
};

struct TModel {
	TOsc FOscs[3];
	TVCF FVCF; 
	TVCA FVCA; 
	TPWM FPWM;
	float FDetune; float FDelay;
	TLFO FLFOs[2];
	float FVCFKeyFollow; float FGlide,FLevel;
  bool FGlideEnable,FPoly,FReleased, FIsQuickReleasing;
  float FStartPitch,FPitch;
	float FModWheel, FPitchBend;
	float VCFGain, VCAModDepth, FADSRGainVCF, FADSRGainVCA, VCAGain;
};

TModel MODEL;
void setParameter(int index,int value); 

#define OSCILLATORCOUNT 2
#define LFOCOUNT 2
TXOscillator FOscillators[OSCILLATORCOUNT];
TXOscillator LFO[LFOCOUNT];
TXSynthBlockADSR FADSRVCA,FADSRVCF;
XMoogFilter FFilter;

int FTICKS;

void bound(float &f)
{
  if (f<-1) f=-1;
  if (f>1)  f = 1;
}

#define SAMPLES_TO_UPDATE 100
void XAddParameter(int param, int min, int max, int val)
{
   OSC_SetParameter(param,ROUND((100.0*(val-min))/(max-min)));  // 0,3,1 = 33      round (
}

void OSC_INIT(uint32_t platform, uint32_t api)
{
  OsInit();
  for (int iosc=0;iosc<OSCILLATORCOUNT;iosc++)
  {
    FOscillators[iosc].SetWaveShape(wsSquare);
  }
  for (int ilfo=0;ilfo<LFOCOUNT;ilfo++)
  {
		LFO[ilfo].SetIsLFO(true);
    LFO[ilfo].SetFrequency(5*SAMPLES_TO_UPDATE);
  }
  FADSRVCA.Release(); 

XAddParameter(CRUMARCC_OSC1WAVE, 0, 3, 0);
  XAddParameter(CRUMARCC_VCAATTACK,0, 100, 10);
  XAddParameter(CRUMARCC_VCADECAY,  0, 100, 80);
  XAddParameter(CRUMARCC_VCASUSTAIN, 0, 100, 80);
  XAddParameter(CRUMARCC_VCARELEASE, 0, 100, 20);
  XAddParameter(CRUMARCC_OSC1LEVEL,  0, 100, 60);
  XAddParameter(CRUMARCC_OSC2WAVE, 0, 3, 0);
  XAddParameter(CRUMARCC_VCFATTACK, 0, 100, 10);
  XAddParameter(CRUMARCC_VCFDECAY, 0, 100, 80);
  XAddParameter(CRUMARCC_VCFSUSTAIN, 0, 100, 80);
  XAddParameter(CRUMARCC_VCFRELEASE, 0, 100, 20);
  XAddParameter(CRUMARCC_OSC2LEVEL,  0, 100, 0);
  XAddParameter(CRUMARCC_CUTOFF,  20, 20000, 10000);
  XAddParameter(CRUMARCC_RESONANCE, 0, 100, 0);
  XAddParameter(CRUMARCC_OUTPUTLEVEL, 0, 100, 100);
  XAddParameter(CRUMARCC_LFO1WAVE, 0, 3, 0);
  XAddParameter(CRUMARCC_LFO1RATE,  0, 20, 10);
  XAddParameter(CRUMARCC_OSC1MODSELECT,  0, 3, 1);
  XAddParameter(CRUMARCC_OSC1MODLEVEL, 0, 100, 50);
  XAddParameter(CRUMARCC_NOISECOLOR, 0, 1, 0);
  XAddParameter(CRUMARCC_NOISELEVEL,  0, 100, 0);
  XAddParameter(CRUMARCC_OSC1FOOT,  0, 3, 2);
  XAddParameter(CRUMARCC_OSC1SEMI,  -12, 12, 0);
  XAddParameter(CRUMARCC_OSC2FOOT,  0, 3, 2);
  XAddParameter(CRUMARCC_OSC2SEMI,  -12, 12, 0);
// TODO..  XAddParameter(CRUMARCC_OSC2DETUNE, 0, 100, 10);
  XAddParameter(CRUMARCC_OSC2MODSELECT, 0, 3, 0);
  XAddParameter(CRUMARCC_OSC2MODLEVEL,0, 100, 0);
  XAddParameter(CRUMARCC_VCFMODSELECT,  0, 3, 0);
  XAddParameter(CRUMARCC_VCFMODLEVEL, 0, 100, 0);
  XAddParameter(CRUMARCC_VCAMODSELECT,  0, 3, 0);
  XAddParameter(CRUMARCC_VCAMODLEVEL, 0, 100, 0);
  XAddParameter(CRUMARCC_VCFLEVEL,  0, 100, 80);
  XAddParameter(CRUMARCC_VCALEVEL,  0, 100, 80);
  XAddParameter(CRUMARCC_VCAMODLEVEL, 0, 100, 0);
  XAddParameter(CRUMARCC_LFO1DELAYENABLE,  0, 1, 0);
  XAddParameter(CRUMARCC_LFO2WAVE, 0, 3, 0 );
  XAddParameter(CRUMARCC_LFO2RATE,  0, 127, 5);
  XAddParameter(CRUMARCC_LFO2DELAYENABLE, 0, 1, 0);
  XAddParameter(CRUMARCC_LFODELAY,  0, 100, 30);
  XAddParameter(CRUMARCC_PWMMODSELECT, 0, 3, 0);
  XAddParameter(CRUMARCC_PWMMODLEVEL, 0, 100, 50);
  XAddParameter(CRUMARCC_GLIDE, 0, 100, 30);
  XAddParameter(CRUMARCC_VCFKEYFOLLOW, 0, 100, 0);
  XAddParameter(CRUMARCC_POLYENABLE, 0, 1, 0);
  XAddParameter(CRUMARCC_GLIDEENABLE, 0, 1, 0);  

	MODEL.FPitchBend = 0.5;
}

float GetOscModDepth(int osc,int lfo)
{
	float result;
	if ((MODEL.FOscs[osc].ModTarget & (1 << lfo)) != 0)
		result = MAX(MODEL.FOscs[osc].ModDepth, MODEL.FModWheel);
	else result = 0;
  return result;
}

float GetOscPulseWidth(int osc)
{
	float result; 
	float t;

	result = 0.5;
	if ((osc == 0) && (MODEL.FOscs[0].Wave == 0))
	{
		if (MODEL.FPWM.ModTarget == 0)
			result = 0.5 - 0.45*MODEL.FPWM.ModDepth; 
        else
		{
			t = 0;
			{ for (int lfo = 0; lfo < 2; ++lfo) 
         if ((MODEL.FPWM.ModTarget & (1 << lfo)) != 0)
      			t = t + MODEL.FLFOs[lfo].value *1.0 / 2; 
      }
			result = 0.5 + 0.45*MODEL.FPWM.ModDepth*t;
		}
	}
	return result;
}

void GetVCA(float & A, float & D, float & S, float & R, float & gain)
{
	A = MODEL.FVCA.Attack;
	D = MODEL.FVCA.Decay;
	S = MODEL.FVCA.Sustain;
	R = MODEL.FVCA.Release;
	gain = MODEL.FVCA.Level;

}
void GetVCF(float & A, float & D, float & S, float & R, float & gain)
{
	A = MODEL.FVCF.Attack;
	D = MODEL.FVCF.Decay;
	S = MODEL.FVCF.Sustain;
	R = MODEL.FVCF.Release;
	gain = MODEL.FVCF.Level;
}

float pitch2Hertz(float pitch)
{
		uint8_t note = pitch;
    uint8_t mod = (note-pitch)*256;
		const float f0 = osc_notehzf(note);
		const float f1 = osc_notehzf(note + 1);
		return clipmaxf(linintf(mod * k_note_mod_fscale, f0, f1), k_note_max_hz);
}

float ModelGetOscFrequency(int osc, float & currPitch)
{
		if (osc == 2) return 10000;
		currPitch = currPitch + (MODEL.FOscs[osc].Foot - 2) * 12 + MODEL.FOscs[osc].Semi;
    
		if (currPitch<12) currPitch = 12;
		if (currPitch>108) currPitch = 108;
		currPitch = currPitch + 16 * (MODEL.FPitchBend - 0.5);
    int FDetune=0; // TODO... 
		if (osc == 0) currPitch = currPitch - FDetune * 1.0 / 2; else currPitch = currPitch + FDetune * 1.0 / 2;

		return pitch2Hertz(currPitch);
}

float GetCutoff(float pitch)
{
   return MODEL.FVCF.Cutoff;
}

float GetResonance()
{
   return MODEL.FVCF.Resonance;
}


float GetVCFModDepth(int lfo)
{
		float result;
		if ((MODEL.FVCF.ModTarget & (1 << lfo)) != 0)
				result = MAX(MODEL.FVCF.ModDepth, MODEL.FModWheel);
		else result = 0;
		return result;
}

float GetVCAModDepth(int lfo)
{
		float result;
		if ((MODEL.FVCA.ModTarget & (1 << lfo)) != 0)
				result = MAX(MODEL.FVCA.ModDepth, MODEL.FModWheel);
		else result = 0;
		return result;
}

float GetOscFrequency(int osc, float & currPitch)
{
	float depth; 
    float ratio;
	float spitch;  

	if (!MODEL.FGlideEnable) ratio=1;  
	else
	{
		float timeToGlide = ABS(MODEL.FPitch - MODEL.FStartPitch) * MODEL.FGlide *1.0 / 12; // in Seconds...
		ratio = FTICKS *1.0 / (timeToGlide * 48000.0);
		if (ratio > 1) ratio=1;
    } 
	spitch = MODEL.FPitch*ratio + MODEL.FStartPitch*(1 - ratio);
	currPitch = spitch;
	return ModelGetOscFrequency(osc, spitch);
}

float calcLFO(float value)
{
	if ((value<0.5)) return 0.1 + 2.9*value * 2; else return 3 + 34 * (value - 0.5);
}

TWaveShape setOscWaveshape(int osc, int value)
{
  static TWaveShape oscwav[4] = { wsSquare, wsSquare,wsTriangle, wsSawUp };
	static TWaveShape lfo1wav[4] = { wsSawUp,  wsSquare,wsSine,wsSawDown };
	static TWaveShape lfo2wav[4] = { wsStaircase, wsSquare,wsSine, wsSAndH };
	int index = ROUND(value*3.0/100);
  switch (osc)
  { case 0: case 1: return oscwav[index];
    case 2:         return wsNoise;
    case 3:         return lfo1wav[index];
		case 4:         return lfo2wav[index];
  }
}


int modtarget(int value)
{
  return ROUND(3.0*value/100);
}

int foot(int value)
{
		return ROUND(3.0 * value / 100);
}

int semi(int value)
{
		return (value-64)/10; 
}


void OSC_SetParameter(uint8_t cc, uint8_t value)  
{
	switch (cc) {
	case CRUMARCC_OSC1WAVE:     MODEL.FOscs[0].Wave = setOscWaveshape(0,value);
		break;
	case CRUMARCC_VCAATTACK:    MODEL.FVCA.Attack = value/100.0;
		break;
	case CRUMARCC_VCADECAY:     MODEL.FVCA.Decay = value/100.0;
		break;
	case CRUMARCC_VCARELEASE:   MODEL.FVCA.Release = value/100.0;
		break;
	case CRUMARCC_VCASUSTAIN:   MODEL.FVCA.Sustain = value/100.0;
		break;
	case CRUMARCC_OSC1LEVEL:    MODEL.FOscs[0].Level = value/100.0;
		break;
	case CRUMARCC_OSC2WAVE:     MODEL.FOscs[1].Wave = setOscWaveshape(1, value);
		break;
	case CRUMARCC_VCFATTACK:    MODEL.FVCF.Attack = value/100.0;
		break;
	case CRUMARCC_VCFDECAY:     MODEL.FVCF.Decay = value/100.0;
		break;
	case CRUMARCC_VCFRELEASE:   MODEL.FVCF.Release = value/100.0;
		break;
	case CRUMARCC_VCFSUSTAIN:   MODEL.FVCF.Sustain = value/100.0;
		break;
	case CRUMARCC_OSC2LEVEL:    MODEL.FOscs[1].Level = value/100.0;
		break;
	case CRUMARCC_CUTOFF:       MODEL.FVCF.Cutoff =50+value*160;
		break;
	case CRUMARCC_RESONANCE:    MODEL.FVCF.Resonance = value/100.0;
		break;
	case CRUMARCC_OUTPUTLEVEL:  MODEL.FLevel = value/100.0;
		break;
	case CRUMARCC_LFO1WAVE:     MODEL.FLFOs[0].Wave = setOscWaveshape(3, value);
		break;
	case CRUMARCC_LFO1RATE:     MODEL.FLFOs[0].speed = calcLFO(value/100.0);
		break;
	case CRUMARCC_OSC1MODSELECT:  MODEL.FOscs[0].ModTarget = modtarget(value);
		break;
	case CRUMARCC_OSC1MODLEVEL: MODEL.FOscs[0].ModDepth = value/100.0;
		break;
	case CRUMARCC_NOISECOLOR:   MODEL.FOscs[2].Wave = setOscWaveshape(2, value);
		break;
	case CRUMARCC_NOISELEVEL:   MODEL.FOscs[2].Level = value/100.0;
		break;
	case CRUMARCC_OSC1FOOT:     MODEL.FOscs[0].Foot = foot(value);
		break;
	case CRUMARCC_OSC1SEMI:     MODEL.FOscs[0].Semi = semi(value);
		break;
	case CRUMARCC_OSC2FOOT:     MODEL.FOscs[1].Foot = foot(value);
		break;
	case CRUMARCC_OSC2SEMI:     MODEL.FOscs[1].Semi = semi(value);
		break;
	case CRUMARCC_OSC2MODSELECT:  MODEL.FOscs[1].ModTarget = modtarget(value);
		break;
	case CRUMARCC_OSC2MODLEVEL: MODEL.FOscs[1].ModDepth = value/100.0;
		break;
	case CRUMARCC_VCFMODSELECT:   MODEL.FVCF.ModTarget = modtarget(value);
		break;
	case CRUMARCC_VCFMODLEVEL:  MODEL.FVCF.ModDepth = value/100.0;
		break;
	case CRUMARCC_VCAMODSELECT:   MODEL.FVCA.ModTarget = modtarget(value);
		break;
	case CRUMARCC_VCAMODLEVEL:  MODEL.FVCA.ModDepth = value/100.0;
		break;
	case CRUMARCC_VCFLEVEL:     MODEL.FVCF.Level = value/100.0;
		break;
	case CRUMARCC_VCALEVEL:     MODEL.FVCA.Level = value/100.0;
		break;
	case CRUMARCC_LFO1DELAYENABLE:    MODEL.FLFOs[0].delay = value!=0;
		break;
	case CRUMARCC_LFO2WAVE:     MODEL.FLFOs[1].Wave = setOscWaveshape(4, value);
		break;
	case CRUMARCC_LFO2RATE:     MODEL.FLFOs[1].speed = calcLFO(value/100.0);
		break;
	case CRUMARCC_LFO2DELAYENABLE:    MODEL.FLFOs[1].delay = value!=0;
		break;
	case CRUMARCC_LFODELAY:     MODEL.FDelay = value/100.0;
		break;
	case CRUMARCC_PWMMODSELECT:   MODEL.FPWM.ModTarget = modtarget(value);
		break;
	case CRUMARCC_PWMMODLEVEL:  MODEL.FPWM.ModDepth = value/100.0;
		break;
	case CRUMARCC_GLIDE:        MODEL.FGlide = value/100.0;
		break;
	case CRUMARCC_GLIDEENABLE:  MODEL.FGlideEnable = value!=0;
		break;
	case CRUMARCC_VCFKEYFOLLOW: MODEL.FVCFKeyFollow = value/100.0;
		break;
	case CRUMARCC_POLYENABLE:         MODEL.FPoly = value!=0;
		break;
	}
}

void updateParameters()
{

	float currPitch;

// LFOs //////////////////////////////////////	 
	 for (int ilfo=0;ilfo<LFOCOUNT;ilfo++) 
   {
			 LFO[ilfo].SetWaveShape(MODEL.FLFOs[ilfo].Wave);
			 LFO[ilfo].SetFrequency(MODEL.FLFOs[ilfo].speed * SAMPLES_TO_UPDATE);  
      MODEL.FLFOs[ilfo].value=LFO[ilfo].getSample();
	 // TODO: LFODelay
   }
// OSC s 
	for (int iosc = 0; iosc < OSCILLATORCOUNT; ++iosc)
	{
		// OSC LFO modulation
		float t=0;
		for (int ilfo = 0; ilfo < 2; ++ilfo)
			t = t + MODEL.FLFOs[ilfo].value * GetOscModDepth(iosc, ilfo) *1.0 / 4;
		// t ligt tussen -0.5 en 0.5 en dat betekent 1 octaaf max...
		FOscillators[iosc].SetWaveShape(MODEL.FOscs[iosc].Wave);
		FOscillators[iosc].SetPulseWidth(GetOscPulseWidth(iosc));
    float freq= GetOscFrequency(iosc, currPitch)*(1 + t);

		FOscillators[iosc].SetFrequency(freq);
	}
  FADSRVCA.SetADSR(MODEL.FVCA.Attack, MODEL.FVCA.Decay, MODEL.FVCA.Sustain, MODEL.FVCA.Release);
	MODEL.FADSRGainVCA= FADSRVCA.Process(SAMPLES_TO_UPDATE);
	FADSRVCF.SetADSR(MODEL.FVCF.Attack, MODEL.FVCF.Decay, MODEL.FVCF.Sustain, MODEL.FVCF.Release);
	MODEL.FADSRGainVCF = FADSRVCF.Process(SAMPLES_TO_UPDATE);
	float t = 0;
	for (int ilfo = 0; ilfo < 2; ++ilfo)
		t = t + MODEL.FLFOs[ilfo].value*GetVCFModDepth(ilfo) *1.0 / 4;
	FFilter.setCutoff(GetCutoff(currPitch)*MODEL.FVCF.Level *(1 + t));
	FFilter.setResonance(GetResonance());

	// VCA LFO & Amp & Volume
	t = 0;
	for (int ilfo = 0; ilfo < 2; ++ilfo)
		t = t + MODEL.FLFOs[ilfo].value*GetVCAModDepth(ilfo) *1.0 / 4;
	MODEL.FVCA.Level = (1 + t);

}

void OSC_CYCLE(const user_osc_param_t * const params,		int32_t *yn,		const uint32_t frames)
{
	 FTICKS++;
   static int totFrames;
   totFrames+=frames;
   if (totFrames>= SAMPLES_TO_UPDATE)
   {
	   updateParameters();
     totFrames=0; 
   } // TODO naming and myos.c
   float vca = MODEL.FADSRGainVCA * MODEL.FVCA.Level;
   if (vca>0)
   {
     for (int i = 0; i<frames; i++)
     {
       float result=0;
       for (int iosc=0;iosc<OSCILLATORCOUNT; iosc++)  
			  result+=FOscillators[iosc].getSample()*MODEL.FOscs[iosc].Level;
       bound(result); 
	     result = FFilter.process(result);
		// VCA LFO & Amp & Volume
	     result = result * vca*(1 + MODEL.FVCA.ModDepth);
       *yn++ = f32_to_q31(result); 
	 }
   }
   else for (int i = 0; i<frames; i++) *yn++=0;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
		MODEL.FReleased = false;
	FTICKS = 0;
  MODEL.FPitch = params->pitch>>8;
  if (!MODEL.FGlideEnable)
			MODEL.FStartPitch = MODEL.FPitch;
	MODEL.FIsQuickReleasing = false;
	FADSRVCA.Trigger();
	FADSRVCF.Trigger();
	updateParameters();	
	FFilter.reset();
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
	MODEL.FReleased = true;
	FADSRVCA.Release();
	FADSRVCF.Release();
}

void OSC_PARAM_NEW(uint16_t index, uint16_t value)
{
		const float valf = param_val_to_f32(value);

		switch (index) {
		case k_user_osc_param_id1:
		case k_user_osc_param_id2:
		case k_user_osc_param_id3:
		case k_user_osc_param_id4:
		case k_user_osc_param_id5:
		case k_user_osc_param_id6:
				break;
		case k_user_osc_param_shape:
				break;
		case k_user_osc_param_shiftshape:
				break;
				break;
		}
}

