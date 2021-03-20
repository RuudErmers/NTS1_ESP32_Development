/**   @file  
     @brief  
*/
#include "UXOscillator.h"
#include "myos.h"
#include "userosc.h"

/* TOscillator */

 
#include "Math.h"

TXOscillator::TXOscillator()
{
  FFrequency = 1000;
  FSAndHLevel =0;
  FStairCaseLevel =0;
  FWaveshape = wsNone;
  FIsLfo =false;
  SetSampleRate(44100);
	FPulseWidth = 0.5;
	FWaveshape = wsTriangle;
  FxPos = 0;
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
{   float result=0; float pinknoise; float whitenoise;
   switch( FWaveshape ) {
    case wsNone:  result =0; break;
    case wsNoise:   {
                 whitenoise = (2 * RANDOM() - 1);
								 pinknoise = FPinky.process();
								 result = whitenoise * NoiseColor + pinknoise * (1 - NoiseColor);
               }
										break;
    case wsSAndH:  result =FSAndHLevel; break;
    case wsStaircase:  result =FStairCaseLevel; break;
    case wsSine:  {
				               float pos = positionRe * angleRe - positionIm * angleIm;
				               positionIm = positionIm * angleRe + positionRe * angleIm;
				               positionRe = pos;
				             result = pos;
                  }
                  break; 
    case wsSquare:  if(  FxPos<0.5 ) result =1;else result =-1; break;
    case wsTriangle:  {
                     if(  FxPos<0.25 ) result =4*FxPos;else if(  FxPos<0.75 ) result =2-4*FxPos;else result =-4+4*FxPos;
                   }break;
    case wsSawUp: case wsSawDown: 
                   {
                     if(  FxPos<0.5 ) result =2*FxPos;else result =-2+2*FxPos;
                     if(  FWaveshape == wsSawDown ) result =-result;
                   }break;
  }
return result;
}

void TXOscillator::SampleRateChanged()
{
  if(  FSampleRate>0 )
    FSampleReci = 1 *1.0/ FSampleRate;
}

#define M_PI 3.141592
void TXOscillator::UpdateAngles()
{
  if (FWaveshape == wsSine)
  {
			angleIm = sin(2 * M_PI *FFrequency * FSampleReci);
			angleRe = cos(2 * M_PI *FFrequency * FSampleReci);
  }
}

void TXOscillator::SetFrequency( float  Value)
{
  if (FFrequency!=Value) 
   {
     FFrequency = Value;
     FDeltaXPos = FSampleReci* FFrequency;
     FDeltaXPosReci =1/FDeltaXPos;
     UpdateAngles();
   }  
}

void TXOscillator::SetWaveShape(TWaveShape v)
{
  if (FWaveshape!=v) 
  {
    FWaveshape = v;
		positionRe = 1;
		positionIm = 0;
    UpdateAngles();
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

/* TXOscillatorBlep */

// http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/

float TXOscillator::PValueAt(float FxPos)
{   float result;

  result =BValueAt(FxPos);
  if(  FIsLfo ) return result;
   switch( FWaveshape ) {
    case wsSawUp: case wsSawDown:  {
                         result =result - PolyBlep( FxPos < 0.5 ? ( FxPos+0.5 ): ( FxPos-0.5 ) );
                         if(  FWaveshape==wsSawDown ) result =-result;
                       }
      case wsSquare:  {
                result =result + PolyBlep( FxPos );
                result =result - PolyBlep( FxPos < 0.5 ? ( FxPos+0.5 ): ( FxPos-0.5 ) );
              }
  }
return result;
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
    FBuffer[sample]= PValueAt(p);
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

float TXOscillator::PolyBlep(float t)
{   float result;
    //Serial.println("PolyBlep:");

    float dt = FDeltaXPos;

    // 0 <= t < 1
    if(  ( t < dt ) )   
    {
        t =t *FDeltaXPosReci;  // t E [0,1] , value 1 -> 0
        t = t-1;
        return t*t;
//        result = 2*t - t*t - 1.0;   - (t*t-2*t+1)
    }
    // -1 < t < 0
    else if(  ( t > 1.0 - dt ) )
    {
        t = ( t - 1.0 ) *FDeltaXPosReci; // t E [-1,0] value 0 -> 1
        t = t+1;
        return t*t;
 //       result = t*t + 2*t + 1.0;
    }
    // 0 otherwise
    else result =0.0;
return result;
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
