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
		int Wave;
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
		int Wave;
		float speed;
		bool delay;
		float value; // current lfo value
};
struct TPWM {
		float ModDepth;
		int ModTarget;
};

#define OSCILLATORCOUNT 2
#define LFOCOUNT 2

struct TModel {
		TOsc FOscs[OSCILLATORCOUNT+1];
		TVCF FVCF;
		TVCA FVCA;
		TPWM FPWM;
		float FDetune; float FDelay;
		TLFO FLFOs[2];
		float FVCFKeyFollow; float FGlide, FLevel;
		bool FGlideEnable, FPoly;
		float FModWheel, FPitchBend;
		float VCFGain, VCAModDepth,  VCAGain;
		void XAddParameter(int param, int min, int max, int val);
		float GetOscModDepth(int osc, int lfo);
		float GetOscPulseWidth(int osc, float lfos[LFOCOUNT]);
		void GetVCA(float & A, float & D, float & S, float & R, float & gain);
		void GetVCF(float & A, float & D, float & S, float & R, float & gain);
		float ModelGetOscFrequency(int osc, float & currPitch);
		void setParameter(int cc, int value);
		float GetCutoff(float pitch);
		float GetVCFModDepth(int lfo);
		float GetVCAModDepth(int lfo);
		void Init();
		int setOscWaveshape(int osc, int value);
};

//#define GLOBAL_LFOS  <- Dan crashed het, ook al doe ik niets met Voice[].LFO

class XCrumarVoice
{
private:
		int FTicks; 
		void updateParameters();
		float FVCALevel;
		uint8_t FStartPitch;
		TXOscillator FOscillators[OSCILLATORCOUNT];
#ifdef GLOBAL_LFOS		
		TXOscillator *LFO;
#else
		TXOscillator LFO[LFOCOUNT];      		
#endif		
		TXSynthBlockADSR FADSRVCA, FADSRVCF;
		XMoogFilter FFilter;
		float GetOscFrequency(int osc, float & currPitch);
public:
    uint8_t FPitch;
		bool FReleased;
		XCrumarVoice() {}
		void NoteOn(int pitch);
		void NoteOff();
		void ProcessAdd(int32_t *yn, const uint32_t frames, bool firstVoice,bool update);
#ifdef GLOBAL_LFOS		
		void Init(TXOscillator *LFOs);
#else
		void Init();
#endif
};

class XCrumarSynth
{
private:
		XCrumarVoice Voices[VOICECOUNT];
#ifdef GLOBAL_LFOS		
		TXOscillator LFO[LFOCOUNT];
#endif		
		int FSamplesToUpdate;
public:
		void NoteOn(int pitch);
		void NoteOff();
		void Process(int32_t *yn, const uint32_t frames);
		void Init();
    void SetParameter(uint8_t cc, uint8_t value); 
		void DoNoteEvent(uint8_t pitch, uint8_t velo);  // extension on Korg framework

};



#endif
