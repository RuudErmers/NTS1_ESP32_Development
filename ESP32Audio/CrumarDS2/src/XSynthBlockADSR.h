#ifndef INCLUDED_UXSYNTHBLOCKS_H
#define INCLUDED_UXSYNTHBLOCKS_H
enum TADSRStage { adsrAttack, adsrDecay, adsrSustain, adsrRelease };
         
struct/*class*/ TXSynthBlockADSR
    
{
protected:
       TADSRStage FADSRStage;
       bool FIsQuickReleasing;
       int FDeltaTicks;
       float FADSRA; float FADSRD; float FADSRR; float deltaA; float deltaD; float S; float deltaR; float R; float FADSRGain; float FADSRGainRelease;
    
protected:
       bool ShouldRelease();
       float CalcAttack(int ticks);
       float CalcDecay(int ticks);
       float CalcRelease(int ticks);
public:
	float Process(int samples);
       void SetADSR(float a, float d, float S, float R);
       void Trigger();
       void Release(bool Quick = false);
       bool IsDone();   
};


#endif//INCLUDED_UXSYNTHBLOCKS_H
//END
