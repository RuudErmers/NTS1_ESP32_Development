#include "XCrumarSynth.h"

void bound(float &f)
{
		if (f<-1) f = -1;
		if (f>1)  f = 1;
}

TModel MODEL;

#define SAMPLES_TO_UPDATE 400
void TModel::XAddParameter(int param, int min, int max, int val)
{
		setParameter(param, ROUND((100.0*(val - min)) / (max - min))); 
}

#ifdef GLOBAL_LFOS		
	void XCrumarVoice::Init(TXOscillator *LFOs)
#else
	void XCrumarVoice::Init()
#endif	
{
  for (int iosc = 0; iosc<OSCILLATORCOUNT; iosc++)
  {
		FOscillators[iosc].SetWaveShape(wsSquare);
  }  
  FOscillators[OSCILLATORCOUNT].SetWaveShape(wsNoise);
#ifdef GLOBAL_LFOS		
    LFO=LFOs;
#else
  for (int ilfo = 0; ilfo<LFOCOUNT; ilfo++)
  {
		LFO[ilfo].SetIsLFO(true);
		LFO[ilfo].SetFrequency(5 * SAMPLES_TO_UPDATE);
  }
#endif		
  NoteOff();
}

void TModel::Init()
{
XAddParameter(CRUMARCC_OSC1WAVE, 0, 100, 5);
XAddParameter(CRUMARCC_VCAATTACK, 0, 100, 10);
XAddParameter(CRUMARCC_VCADECAY, 0, 100, 80);
XAddParameter(CRUMARCC_VCASUSTAIN, 0, 100, 80);
XAddParameter(CRUMARCC_VCARELEASE, 0, 100, 20);
XAddParameter(CRUMARCC_OSC1LEVEL, 0, 100, 60);
XAddParameter(CRUMARCC_OSC2WAVE, 0, 100, 7);
XAddParameter(CRUMARCC_VCFATTACK, 0, 100, 10);
XAddParameter(CRUMARCC_VCFDECAY, 0, 100, 80);
XAddParameter(CRUMARCC_VCFSUSTAIN, 0, 100, 80);
XAddParameter(CRUMARCC_VCFRELEASE, 0, 100, 20);
XAddParameter(CRUMARCC_OSC2LEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_CUTOFF, 0, 100, 70);
XAddParameter(CRUMARCC_RESONANCE, 0, 100, 0);
XAddParameter(CRUMARCC_OUTPUTLEVEL, 0, 100, 100);
XAddParameter(CRUMARCC_LFO1WAVE, 0, 3, 0);
XAddParameter(CRUMARCC_LFO1RATE, 0, 20, 10);
XAddParameter(CRUMARCC_OSC1MODSELECT, 0, 3, 1);
XAddParameter(CRUMARCC_OSC1MODLEVEL, 0, 100, 50);
XAddParameter(CRUMARCC_NOISECOLOR, 0, 1, 0);
XAddParameter(CRUMARCC_NOISELEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_OSC1FOOT, 0, 3, 2);
XAddParameter(CRUMARCC_OSC1SEMI, -12, 12, 0);
XAddParameter(CRUMARCC_OSC2FOOT, 0, 3, 2);
XAddParameter(CRUMARCC_OSC2SEMI, -12, 12, 0);
// TODO..  XAddParameter(CRUMARCC_OSC2DETUNE, 0, 100, 10);
XAddParameter(CRUMARCC_OSC2MODSELECT, 0, 3, 0);
XAddParameter(CRUMARCC_OSC2MODLEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_VCFMODSELECT, 0, 3, 0);
XAddParameter(CRUMARCC_VCFMODLEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_VCAMODSELECT, 0, 3, 0);
XAddParameter(CRUMARCC_VCAMODLEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_VCFLEVEL, 0, 100, 80);
XAddParameter(CRUMARCC_VCALEVEL, 0, 100, 80);
XAddParameter(CRUMARCC_VCAMODLEVEL, 0, 100, 0);
XAddParameter(CRUMARCC_LFO1DELAYENABLE, 0, 1, 0);
XAddParameter(CRUMARCC_LFO2WAVE, 0, 3, 0);
XAddParameter(CRUMARCC_LFO2RATE, 0, 127, 5);
XAddParameter(CRUMARCC_LFO2DELAYENABLE, 0, 1, 0);
XAddParameter(CRUMARCC_LFODELAY, 0, 100, 30);
XAddParameter(CRUMARCC_PWMMODSELECT, 0, 3, 0);
XAddParameter(CRUMARCC_PWMMODLEVEL, 0, 100, 50);
XAddParameter(CRUMARCC_GLIDE, 0, 100, 30);
XAddParameter(CRUMARCC_VCFKEYFOLLOW, 0, 100, 0);
XAddParameter(CRUMARCC_POLYENABLE, 0, 1, 0);
XAddParameter(CRUMARCC_GLIDEENABLE, 0, 1, 0);

FPitchBend = 0.5;
}

float TModel::GetOscModDepth(int osc, int lfo)
{
		float result;
		if ((FOscs[osc].ModTarget & (1 << lfo)) != 0)
				result = MAX(FOscs[osc].ModDepth, FModWheel);
		else result = 0;
		return result;
}

float TModel::GetOscPulseWidth(int osc,float lfos[LFOCOUNT])
{
		float result;
		float t;

		result = 0.5;
		if ((osc == 0) && (FOscs[0].Wave == wsSquare))
		{
				if (FPWM.ModTarget == 0)
						result = 0.5 - 0.45*FPWM.ModDepth;
				else
				{
						t = 0;
						{ for (int lfo = 0; lfo < 2; ++lfo)
								if ((FPWM.ModTarget & (1 << lfo)) != 0)
										t = t + lfos[lfo] *1.0 / 2;
						}
						result = 0.5 + 0.45*FPWM.ModDepth*t;
				}
		}
		return result;
}

void TModel::GetVCA(float & A, float & D, float & S, float & R, float & gain)
{
		A = FVCA.Attack;
		D = FVCA.Decay;
		S = FVCA.Sustain;
		R = FVCA.Release;
		gain = FVCA.Level;

}
void TModel::GetVCF(float & A, float & D, float & S, float & R, float & gain)
{
		A = FVCF.Attack;
		D = FVCF.Decay;
		S = FVCF.Sustain;
		R = FVCF.Release;
		gain = FVCF.Level;
}

float pitch2Hertz(float pitch)
{
		uint8_t note = pitch;
		uint8_t mod = (note - pitch) * 256;
		const float f0 = osc_notehzf(note);
		const float f1 = osc_notehzf(note + 1);
		return clipmaxf(linintf(mod * k_note_mod_fscale, f0, f1), k_note_max_hz);
}

float TModel::ModelGetOscFrequency(int osc, float & currPitch)
{
		if (osc == OSCILLATORCOUNT) return 10000;
		currPitch = currPitch + (FOscs[osc].Foot - 2) * 12 + FOscs[osc].Semi;

		if (currPitch<12) currPitch = 12;
		if (currPitch>108) currPitch = 108;
		currPitch = currPitch + 16 * (FPitchBend - 0.5);
		int FDetune = 0.05; // TODO... 
		if (osc %2 == 0) currPitch = currPitch - FDetune * 1.0 / 2; else currPitch = currPitch + FDetune * 1.0 / 2;

		return pitch2Hertz(currPitch);
}

float TModel::GetCutoff(float pitch)
{
		return FVCF.Cutoff;
}

float TModel::GetVCFModDepth(int lfo)
{
		float result;
		if ((FVCF.ModTarget & (1 << lfo)) != 0)
				result = MAX(FVCF.ModDepth, FModWheel);
		else result = 0;
		return result;
}

float TModel::GetVCAModDepth(int lfo)
{
		float result;
		if ((FVCA.ModTarget & (1 << lfo)) != 0)
				result = MAX(FVCA.ModDepth, FModWheel);
		else result = 0;
		return result;
}

float XCrumarVoice::GetOscFrequency(int osc, float & currPitch)
{
		float depth;
		float ratio;
		float spitch;

		if (!MODEL.FGlideEnable) ratio = 1;
		else
		{
				float timeToGlide = ABS(FPitch - FStartPitch) * (MODEL.FGlide) *1.0 / 12; // in Seconds...
				ratio = FTicks * 1.0 / (timeToGlide * 48000.0);
				if (ratio > 1) ratio = 1;
		}
		spitch = FPitch*ratio + FStartPitch*(1 - ratio);
		currPitch = spitch;
 		return MODEL.ModelGetOscFrequency(osc, spitch);   
}

float calcLFO(float value)
{
		if ((value<0.5)) return 0.1 + 2.9*value * 2; else return 3 + 34 * (value - 0.5);
}

int TModel::setOscWaveshape(int osc, int value)
{
		static TWaveShape lfo1wav[4] = { wsSawUp,  wsSquare,wsSine,wsSawDown };
		static TWaveShape lfo2wav[4] = { wsStaircase, wsSquare,wsSine, wsSAndH };
		int index = ROUND(value*3.0 / 100);
		if (osc<OSCILLATORCOUNT) return value;
		switch (osc)
		{
			default:         return wsNoise;
		case OSCILLATORCOUNT+1:         return lfo1wav[index];
		case OSCILLATORCOUNT+2:         return lfo2wav[index];
		}
}


int modtarget(int value)
{
		return ROUND(3.0*value / 100);
}

int foot(int value)
{
		return ROUND(3.0 * value / 100);
}

int semi(int value)
{
		return (value - 64) / 10;
}


void TModel::setParameter(int index, int value)
{
		switch (index) {
		case CRUMARCC_OSC1WAVE:     FOscs[0].Wave = setOscWaveshape(0, value);
				break;
		case CRUMARCC_VCAATTACK:    FVCA.Attack = value / 100.0;
				break;
		case CRUMARCC_VCADECAY:     FVCA.Decay = value / 100.0;
				break;
		case CRUMARCC_VCARELEASE:   FVCA.Release = value / 100.0;
				break;
		case CRUMARCC_VCASUSTAIN:   FVCA.Sustain = value / 100.0;
				break;
		case CRUMARCC_OSC1LEVEL:    FOscs[0].Level = value / 100.0;                    
				break;
		case CRUMARCC_OSC2WAVE:     FOscs[1].Wave = setOscWaveshape(1, value);
				break;
		case CRUMARCC_VCFATTACK:    FVCF.Attack = value / 100.0;
				break;
		case CRUMARCC_VCFDECAY:     FVCF.Decay = value / 100.0;
				break;
		case CRUMARCC_VCFRELEASE:   FVCF.Release = value / 100.0;
				break;
		case CRUMARCC_VCFSUSTAIN:   FVCF.Sustain = value / 100.0;
				break;
		case CRUMARCC_OSC2LEVEL:    FOscs[1].Level = value / 100.0;
				break;
		case CRUMARCC_CUTOFF:       FVCF.Cutoff = value/100;
				break;
		case CRUMARCC_RESONANCE:    FVCF.Resonance = value / 100.0;
				break;
		case CRUMARCC_OUTPUTLEVEL:  FLevel = value / 100.0;
				break;
		case CRUMARCC_LFO1WAVE:     FLFOs[0].Wave = setOscWaveshape(OSCILLATORCOUNT+1, value);
				break;
		case CRUMARCC_LFO1RATE:     FLFOs[0].speed = calcLFO(value / 100.0);
				break;
		case CRUMARCC_OSC1MODSELECT:  FOscs[0].ModTarget = modtarget(value);
				break;
		case CRUMARCC_OSC1MODLEVEL: FOscs[0].ModDepth = value / 100.0;
				break;
		case CRUMARCC_NOISECOLOR:   FOscs[2].Wave = setOscWaveshape(OSCILLATORCOUNT, value);
				break;
		case CRUMARCC_NOISELEVEL:   FOscs[2].Level = value / 100.0;
				break;
		case CRUMARCC_OSC1FOOT:     DEBUG("Foot Assign: %d\n", (int)value); FOscs[0].Foot = foot(value);
				break;
		case CRUMARCC_OSC1SEMI:     FOscs[0].Semi = semi(value);
				break;
		case CRUMARCC_OSC2FOOT:     FOscs[1].Foot = foot(value);
				break;
		case CRUMARCC_OSC2SEMI:     FOscs[1].Semi = semi(value);
				break;
		case CRUMARCC_OSC2MODSELECT:  FOscs[1].ModTarget = modtarget(value);
				break;
		case CRUMARCC_OSC2MODLEVEL: FOscs[1].ModDepth = value / 100.0;
				break;
		case CRUMARCC_VCFMODSELECT:   FVCF.ModTarget = modtarget(value);
				break;
		case CRUMARCC_VCFMODLEVEL:  FVCF.ModDepth = value / 100.0;
				break;
		case CRUMARCC_VCAMODSELECT:   FVCA.ModTarget = modtarget(value);
				break;
		case CRUMARCC_VCAMODLEVEL:  FVCA.ModDepth = value / 100.0;
				break;
		case CRUMARCC_VCFLEVEL:     FVCF.Level = value / 100.0;
				break;
		case CRUMARCC_VCALEVEL:     FVCA.Level = value / 100.0;
				break;
		case CRUMARCC_LFO1DELAYENABLE:    FLFOs[0].delay = value != 0;
				break;
		case CRUMARCC_LFO2WAVE:     FLFOs[1].Wave = setOscWaveshape(OSCILLATORCOUNT+2, value);
				break;
		case CRUMARCC_LFO2RATE:     FLFOs[1].speed = calcLFO(value / 100.0);
				break;
		case CRUMARCC_LFO2DELAYENABLE:    FLFOs[1].delay = value != 0;
				break;
		case CRUMARCC_LFODELAY:     FDelay = value / 100.0;
				break;
		case CRUMARCC_PWMMODSELECT:   FPWM.ModTarget = modtarget(value);
				break;
		case CRUMARCC_PWMMODLEVEL:  FPWM.ModDepth = value / 100.0;
				break;
		case CRUMARCC_GLIDE:        FGlide = value / 100.0;
				break;
		case CRUMARCC_GLIDEENABLE:  FGlideEnable = value != 0;
				break;
		case CRUMARCC_VCFKEYFOLLOW: FVCFKeyFollow = value / 100.0;
				break;
		case CRUMARCC_POLYENABLE:         FPoly = value != 0;
				break;
		}
}

void XCrumarVoice::updateParameters()
{

		float currPitch;
    float lfos[LFOCOUNT];

		// LFOs //////////////////////////////////////	 
		for (int ilfo = 0; ilfo<LFOCOUNT; ilfo++)
		{
				lfos[ilfo]=LFO[ilfo].getSample();
#ifndef GLOBAL_LFOS
				LFO[ilfo].SetWaveShape(MODEL.FLFOs[ilfo].Wave);
				LFO[ilfo].SetFrequency(MODEL.FLFOs[ilfo].speed * SAMPLES_TO_UPDATE);
				lfos[ilfo] = LFO[ilfo].getSample();
#endif				
				lfos[ilfo] = 0; // LFO[ilfo].Value;
				// TODO: LFODelay
		}
		// OSC s 
		for (int iosc = 0; iosc < OSCILLATORCOUNT; ++iosc)
		{
				// OSC LFO modulation
				float t = 0;
				for (int ilfo = 0; ilfo < 2; ++ilfo)
						t = t + lfos[ilfo] * MODEL.GetOscModDepth(iosc, ilfo) *1.0 / 4;
				// t ligt tussen -0.5 en 0.5 en dat betekent 1 octaaf max...
				FOscillators[iosc].SetWaveShape(MODEL.FOscs[iosc].Wave);
				FOscillators[iosc].SetPulseWidth(MODEL.GetOscPulseWidth(iosc,lfos));
				float freq = GetOscFrequency(iosc, currPitch)*(1 + t);

				FOscillators[iosc].SetFrequency(freq);
		}
		FADSRVCF.SetADSR(MODEL.FVCF.Attack, MODEL.FVCF.Decay, MODEL.FVCF.Sustain, MODEL.FVCF.Release);
		FADSRVCF.Process(SAMPLES_TO_UPDATE);
		float t = 0;
		for (int ilfo = 0; ilfo < 2; ++ilfo)
				t = t + lfos[ilfo]* MODEL.GetVCFModDepth(ilfo) *1.0 / 4;
        t+=MODEL.GetCutoff(currPitch);
		t+=FADSRVCF.Value*MODEL.FVCF.Level;
		if (t<0) t=0;
		if (t>1) t=1;
		FFilter.setCutoff(100+18000*t);				
		FFilter.setResonance(MODEL.FVCF.Resonance);

		// VCA LFO & Amp & Volume
		FADSRVCA.SetADSR(MODEL.FVCA.Attack, MODEL.FVCA.Decay, MODEL.FVCA.Sustain, MODEL.FVCA.Release);  // TODO where is gain?
		FADSRVCA.Process(SAMPLES_TO_UPDATE);
		t = 0;
		for (int ilfo = 0; ilfo < 2; ++ilfo)
				t = t + lfos[ilfo]* MODEL.GetVCAModDepth(ilfo) *1.0 / 4;
		FVCALevel = (1 + t)*MODEL.FVCA.Level*FADSRVCA.Value;

}

void XCrumarVoice::ProcessAdd(int32_t *yn, const uint32_t frames,bool firstVoice, bool update)
{
		FTicks++;
		if (update) updateParameters();
// Dit wordt positief...		Serial.println("VCA: "+(String)FADSRVCA.Value);

		if ((FVCALevel>0) or firstVoice) 
		{
				for (int i = 0; i<frames; i++)
				{
						float result = 0;
						if (FVCALevel>0)
						{
						  for (int iosc = 0; iosc<OSCILLATORCOUNT; iosc++)  // +1 => Noise
						  		result += FOscillators[iosc].getSample()*MODEL.FOscs[iosc].Level;
						  // FFilter bounds this... bound(result);
						  result = FFilter.process(result);
						  result = result * FVCALevel;
						}
						if (firstVoice)     
						  (*yn) = f32_to_q31(result);
						else  
						  (*yn) += f32_to_q31(result);
            			  yn++; // niet te moeilijk maken hierboven.... 
				} 
		}		
		if ((FADSRVCA.Value<=0) && !FReleased)
		{
//			Serial.println("Releasing Voice...") ;
			FReleased=true;
		}	
}

void XCrumarVoice::NoteOn(int pitch)
{
		FTicks = 0;
		FPitch = pitch;
		if (!MODEL.FGlideEnable)
				FStartPitch = FPitch;
		FADSRVCA.Trigger();
		Serial.println("VCA Start 1: "+(String)FADSRVCA.Value);		
		FADSRVCF.Trigger();
		updateParameters();
		Serial.println("VCA Start 2: "+(String)FADSRVCA.Value);		
		FFilter.reset();
		FReleased = false;		
		Serial.println("Go!: ");		
}

void XCrumarVoice::NoteOff()
{
		FADSRVCA.Release();
		FADSRVCF.Release();
}

void XCrumarSynth::NoteOn(int pitch)  // From NTS1, so this is the Mono Synth
{
   Voices[0].NoteOn(pitch>>8);
}

void XCrumarSynth::NoteOff()
{
   Voices[0].NoteOff();
}

void XCrumarSynth::Init()
{
		OsInit();
    MODEL.Init();	
#ifdef GLOBAL_LFOS		
          for (int ilfo = 0; ilfo<LFOCOUNT; ilfo++)
  		  {
			  LFO[ilfo].SetIsLFO(true);
			  LFO[ilfo].SetFrequency(5 * SAMPLES_TO_UPDATE);
			  LFO[ilfo].getSample();
  		  }
#endif
		for (int ivoc = 0; ivoc<VOICECOUNT; ivoc++)
		{
#ifdef GLOBAL_LFOS		
		  Voices[ivoc].Init(LFO);
#else				
  		  Voices[ivoc].Init();
#endif				
		}
    FSamplesToUpdate=0;
}

void XCrumarSynth::SetParameter(uint8_t cc, uint8_t value)
{
  MODEL.setParameter(cc,value);
}

void XCrumarSynth::Process(int32_t *yn, const uint32_t frames)
{
		FSamplesToUpdate += frames;
		bool FUpdate=FSamplesToUpdate >= SAMPLES_TO_UPDATE;
		if (FUpdate) 
		{
		   FSamplesToUpdate = 0;
#ifdef GLOBAL_LFOS
			for (int ilfo = 0; ilfo<LFOCOUNT; ilfo++) 
			{
		   		LFO[ilfo].SetWaveShape(MODEL.FLFOs[ilfo].Wave);
		   		LFO[ilfo].SetFrequency(MODEL.FLFOs[ilfo].speed * SAMPLES_TO_UPDATE);
				LFO[ilfo].getSample();
			}	
#endif		
		}
		bool firstVoice=true;
		for (int ivoc = 0; ivoc<VOICECOUNT; ivoc++)
		{
				if (!(Voices[ivoc].FReleased))
				{
                   Voices[ivoc].ProcessAdd(yn,frames,firstVoice,FUpdate);
				   firstVoice=false;
				}
		}		   
		if (firstVoice) // no voices 
		  for (int i = 0; i<frames; i++) yn[i]=0;

}

void XCrumarSynth::DoNoteEvent(uint8_t pitch, uint8_t velo)
{

  int p =0;
	for (int ivoc = 0; ivoc<VOICECOUNT; ivoc++)
  {
		if ((Voices[ivoc].FPitch==pitch) &&  !Voices[ivoc].FReleased)
        {
			
          if (velo>0) Voices[ivoc].NoteOn(pitch);
          else        
              Voices[ivoc].NoteOff();
          return; 
        }
        else if (Voices[ivoc].FReleased) { p=ivoc;break; }
  }
	if (velo>0)
      Voices[p].NoteOn(pitch); 
	 
}
