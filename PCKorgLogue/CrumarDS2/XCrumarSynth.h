#ifndef __CrumarSynth_H__
#define __CrumarSynth_H__

#include "userosc.h"
#include "UXOscillator.h"
#include "XSynthBlockADSR.h"
#include "XMoogFilter.h"
#include "myos.h"
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
		float FVCFKeyFollow; float FGlide, FLevel;
		bool FGlideEnable, FPoly, FReleased, FIsQuickReleasing;
		float FStartPitch, FPitch;
		float FModWheel, FPitchBend;
		float VCFGain, VCAModDepth, FADSRGainVCF, FADSRGainVCA, VCAGain;
};

#define OSCILLATORCOUNT 2
#define LFOCOUNT 2

class XCrumarSynth
{
private:
		int FTICKS;
		TModel MODEL;
  void updateParameters();
	TXOscillator FOscillators[OSCILLATORCOUNT];
	TXOscillator LFO[LFOCOUNT];
	TXSynthBlockADSR FADSRVCA, FADSRVCF;
	XMoogFilter FFilter;
	void XAddParameter(int param, int min, int max, int val);
	float GetOscModDepth(int osc, int lfo);
	float GetOscPulseWidth(int osc);
	void GetVCA(float & A, float & D, float & S, float & R, float & gain);
	void GetVCF(float & A, float & D, float & S, float & R, float & gain);
	float ModelGetOscFrequency(int osc, float & currPitch);
	float GetCutoff(float pitch);
	float GetVCFModDepth(int lfo);
	float GetVCAModDepth(int lfo);
	float GetOscFrequency(int osc, float & currPitch);
	TWaveShape setOscWaveshape(int osc, int value);

public:
  XCrumarSynth() {}
  void NoteOn(int pitch);
	void NoteOff();
	void Init();
	void Process(int32_t *yn, const uint32_t frames);
  void setParameter(int cc, int value);

};


#endif
