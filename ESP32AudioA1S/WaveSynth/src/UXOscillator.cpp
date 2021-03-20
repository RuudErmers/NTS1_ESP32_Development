/**   @file  
     @brief  
*/
#include "UXOscillator.h"
#include "myos.h"
#include "userosc.h"
#include "waves.h"

/* TOscillator */

TXOscillator::TXOscillator()
{
  FFrequency = 1000;
  FSAndHLevel =0;
  FStairCaseLevel =0;
  FWaveshape = wsNone;
  FIsLfo =false;
  SetSampleRate(44100);
	SetPulseWidth(0.5);
	SetWaveShape(20);
  FxPos = 0;
  Value=0;
  Process();
}

void TXOscillator::OnZeroCrossed()
{
  if (FWaveshape == wsSAndH)
  {
    FSAndHLevel =2*RANDOM()-1;
    
  }
	if (FWaveshape == wsStaircase)
	{
			FStairCaseLevel = FStairCaseLevel - 1 * 1.0 / 6;
			if ((FStairCaseLevel <= -1)) FStairCaseLevel = 1;
	}

}

float TXOscillator::BValueAt(float FxPos)
{   float pinknoise; float whitenoise;
   switch( FWaveshape ) {
    case wsNone:  return 0;
    case wsNoise: 
                 whitenoise = (2 * RANDOM() - 1);
								 pinknoise = FPinky.process();
								 return whitenoise * NoiseColor + pinknoise * (1 - NoiseColor);
    case wsSAndH:  return FSAndHLevel; 
    case wsStaircase:  return FStairCaseLevel; 
   }
   int index= FxPos*(FWaveTable.size-1);
   return FWaveTable.wave[index];
}

void TXOscillator::SampleRateChanged()
{
  if(  FSampleRate>0 )
    FSampleReci = 1 *1.0/ FSampleRate;
}

void TXOscillator::SetFrequency( float  Value)
{
  if (FFrequency!=Value) 
   {
     FFrequency = Value;
     FDeltaXPos = FSampleReci* FFrequency;
   }  
}

void TXOscillator::SetWaveShape(int v)
{
  if ((FWaveshape!=v) && (v<NR_WAVES)) 
  {
    FWaveshape = v;
    FWaveTable=wavetables[v];
  }  
}

void TXOscillator::SetSampleRate( float  Value)
{
 if(  FSampleRate != Value )
  {
   FSampleRate = Value;
   SampleRateChanged();
  }
}

void TXOscillator::Process()  
{ 
  for (int sample=0;sample<BUFFERSIZE;sample++)
  {
    FxPos =FxPos+ FDeltaXPos;
    if(  FxPos>=1 )
      {
        FxPos =FxPos-1;
      OnZeroCrossed();
      }
    float p=FxPos;
    if (FWaveshape==wsSquare)  p= PulseWidthAdjusted(p);
    FBuffer[sample]= BValueAt(p);
  }
  FBufferP=0;
}

float TXOscillator::PulseWidthAdjusted(float FxPos)
{   float result;
  if(  FxPos<=FPulseWidth )
    result =FxPos*0.5 *FPulseWidthReci1;
  else
    result =0.5+ 0.5*( FxPos-FPulseWidth ) *FPulseWidthReci2;
return result;
}

void TXOscillator::SetPulseWidth(float Value)
{
  if(  Value<0.05 ) Value =0.05;
  if(  Value>0.95 ) Value =0.95;
  FPulseWidth =Value;
  FPulseWidthReci1 = 1.0/ FPulseWidth;
  FPulseWidthReci2 = 1.0/ (1-FPulseWidth);  
}

float TXOscillator::getSample()
{
  if (FBufferP == 32) Process();
  Value=FBuffer[FBufferP++];
  return Value;
}

float XPinky::process()
{
				float result;
				float ur1;

				static float const pA[5] = { 0.23980 , 0.18727 , 0.1638 , 0.194685 , 0.214463 };
				static float const pSUM[5] = { 0.00198 , 0.01478 , 0.06378 , 0.23378 , 0.91578 };

				ur1 = RANDOM();
				if ((ur1 <= pSUM[0])) FContribution[0] = RANDOM() * pA[0]; else
						if ((ur1 <= pSUM[1])) FContribution[1] = RANDOM() * pA[1]; else
								if ((ur1 <= pSUM[2])) FContribution[2] = RANDOM() * pA[2]; else
										if ((ur1 <= pSUM[3])) FContribution[3] = RANDOM() * pA[3]; else
												if ((ur1 <= pSUM[4])) FContribution[4] = RANDOM() * pA[4];
				result = (FContribution[0] + FContribution[1] + FContribution[2] + FContribution[3] + FContribution[4]);
				return result;
}
//END
