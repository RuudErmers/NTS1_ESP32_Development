#ifndef INCLUDED_UXOSCILLATOR_H
#define INCLUDED_UXOSCILLATOR_H
 

  enum TWaveShape { wsNone, wsSine, wsSquare, wsNoise, wsTriangle, wsSawUp, wsSawDown, wsStaircase, wsSAndH };

	class XPinky {
	public:
			XPinky() { for (int i = 0; i<5; i++)  FContribution[i] = 0; }
			float process();
	private:
			float FContribution[5];
	};
 
#define BUFFERSIZE 32 
class TXOscillator
{
protected:
    float FSampleRate;
    float FSampleReci;
    float FFrequency;
    TWaveShape FWaveshape;
    float FSAndHLevel; float FStairCaseLevel;
    bool FIsLfo;
		XPinky FPinky;
		float    positionRe, positionIm, angleRe, angleIm,FPulseWidthReci1,FPulseWidthReci2;
    int FPitch;

  
protected:
    virtual void SampleRateChanged(); 
    void OnZeroCrossed();
    float BValueAt(float FxPos); // FxPos: 0..1
    /*property*/ float NoiseColor;    // 0..1 
		void UpdateAngles();
		float FxPos;
		float FPulseWidth;
        float FDeltaXPos,FDeltaXPosReci;
		float PulseWidthAdjusted(float FxPos);
		void SetVoices(int voices); // FxPos: 0..1
		float FBuffer[BUFFERSIZE];
		int FBufferP;
		void Process();
		float PValueAt(float FxPos); // FxPos: 0..1
		float PolyBlep(float t);

public:
    TXOscillator(); 
    TWaveShape WaveShape()     { return FWaveshape; }
    void SetWaveShape( TWaveShape v );
    float Frequency()     { return FFrequency; }
    float SampleRate()     { return FSampleRate; }
    void SetFrequency( float  Value);
    void SetSampleRate( float  Value);
    void SetNoiseColor(float value) { NoiseColor = value;}
    void SetIsLFO(bool lfo) {		FIsLfo=lfo;}

    float getSample();
		float PulseWidth() { return FPulseWidth; }
		void SetPulseWidth(float Value);
    float Value;
};




#endif//INCLUDED_UXOSCILLATOR_H
//END
