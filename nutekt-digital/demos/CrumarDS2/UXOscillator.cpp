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
     FPolyBlepDt = FSampleReci* FFrequency;
     FPolyBlepDtReci =1/FPolyBlepDt;
     UpdateAngles();
   }  
}

void TXOscillator::SetPitch(int pitch)  // pitch: <semi><frac> both 8 bits
{
  if (pitch!=FPitch)
  {
    FPitch=pitch;
    float f0= osc_notehzf(pitch >> 8);
		float f1 = osc_notehzf((pitch >> 8)+1);
    SetFrequency(f0+(f1-f0)*(pitch & 0xFF )/255.0);
		positionRe=1;
		positionIm=0;
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

float fmod(float s)
{ float result;
  result =s-trunc( s ); 
  return result;
}

float TXOscillator::PValueAt(float FxPos)
{   float result;

  result =BValueAt(FxPos);
  if(  FIsLfo ) return result;
   switch( FWaveshape ) {
    case wsSawUp: case wsSawDown:  {
                         result =result - PolyBlep( fmod( FxPos+0.5 ) );
                         if(  FWaveshape==wsSawDown ) result =-result;
                       }
      case wsSquare:  {
                result =result + PolyBlep( FxPos );
                result =result - PolyBlep( fmod( FxPos + 1-0.5 ) );
              }
  }
return result;
}


void TXOscillator::Process()  
{ 
  for (int sample=0;sample<BUFFERSIZE;sample++)
  {
    FxPos =FxPos+ FSampleReci* FFrequency;
    if(  FxPos>=1 )
      {
        FxPos =FxPos-1;
        OnZeroCrossed();
      }
    FBuffer[sample]= PValueAt(FxPos);
  }
  FBufferP=0;
}

float TXOscillator::PulseWidthAdjusted(float FxPos)
{   float result;
  if(  FxPos<=FPulseWidth )
    result =FxPos*0.5 *1.0/ FPulseWidth;else
    result =0.5+ 0.5*( FxPos-FPulseWidth ) *1.0/ ( 1 - FPulseWidth );
return result;
}

float TXOscillator::VValueAt(float FxPos)
{   float result; float v;

  if(  FIsLfo ) result =PValueAt(FxPos); else
   switch( FWaveshape ) {
    case wsSquare: 
             {
               if(  FxPos<FPulseWidth ) v =1;else v =-1;
               // Polyblep is 0 everywhere except around 0: 0->t : -1->0 and 1-t,1 : 1 -> 0
               result =v + PolyBlep( FxPos );
               result =result - PolyBlep( fmod( FxPos+1-FPulseWidth ) );
             }
    case wsSawUp: case wsSawDown: 
             {
               if(  FxPos<FPulseWidth ) v =FxPos*1.0/FPulseWidth;else v =-1+( FxPos-FPulseWidth )*1.0/( 1-FPulseWidth );
               result =v - PolyBlep( fmod( FxPos+1-FPulseWidth ) );
              if(  FWaveshape == wsSawDown ) result =-result;
             }
    default:     result =PValueAt( PulseWidthAdjusted( FxPos ) );
  }
return result;
}

void TXOscillator::SetPulseWidth(float Value)
{
  if(  Value<0.05 ) Value =0.05;
  if(  Value>0.95 ) Value =0.95;
  FPulseWidth =Value;
}


float TXOscillator::getSample()
{
  if (FBufferP == 32) Process();
  return FBuffer[FBufferP++];
}

float TXOscillator::PolyBlep(float t)
{   float result;

    //return 0;  // TODO !! dit moet weg..
    float dt = FPolyBlepDt;
    if(  dt==0 ) { result =0; return result; }

    // 0 <= t < 1
    if(  ( t < dt ) )
    {
        t =t *FPolyBlepDtReci;
        result = 2*t - t*t - 1.0;
    }
    // -1 < t < 0
    else if(  ( t > 1.0 - dt ) )
    {
        t = ( t - 1.0 ) *FPolyBlepDtReci;
        result = t*t + 2*t + 1.0;
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
