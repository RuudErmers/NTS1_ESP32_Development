/**   @file  
     @brief  
*/
#include "XSynthBlockADSR.h"



/* TXSynthParameter */
// ADSR were changed at 25082018

 
#include "Math.h"
#include "MyOS.h"
float _f(float x, float attack) {   float result;  if(  x ==0 ) result =0;else result = 1 *1.0/ ( x*x*attack ); return result; }

void TXSynthBlockADSR::SetADSR(float a, float d, float S, float R)
{
 static float const MAXTIME_ATTACK = 10L*44100;
 static float const MAXTIME_DECAY = 5L*44100;
 static float const MAXTIME_RELEASE = 10L*44100;

  this->deltaA =_f( a, MAXTIME_ATTACK );
  this->deltaD =_f( d, MAXTIME_DECAY );
  this->S =S;
  this->deltaR =_f( R, MAXTIME_RELEASE );
  this->R =R;
}

bool TXSynthBlockADSR::ShouldRelease()
{   bool result;
  result =true;
return result;
}

bool TXSynthBlockADSR::IsDone()
{   bool result;
  result =FADSRGain <=0;
return result;
}


float TXSynthBlockADSR::CalcAttack(int ticks)
{   float result;
  FADSRA =FADSRA+deltaA*ticks;
  if(  deltaA!=0 )
    result = FADSRA;else
    result =1;
return result;
}
float TXSynthBlockADSR::CalcDecay(int ticks)
{   float result;
  FADSRD =FADSRD+deltaD*ticks;
  if(  deltaD!=0 )
    result = FADSRD;else
    result =1;
return result;
}
float TXSynthBlockADSR::CalcRelease(int ticks)
{   float result;
  FADSRR =FADSRR+deltaR*ticks;
  if(  deltaR!=0 )
    result = FADSRR;else
    result =1;
return result;
}

float TXSynthBlockADSR::Process(int samples) // New Construction: Implemented Full;

{
  ++FDeltaTicks;
  if(  FIsQuickReleasing && ShouldRelease() )
      FADSRGain =MAX( 0, FADSRGain-0.0003*samples );else
    {
       switch( FADSRStage ) {
        case adsrAttack:  {
                        FADSRGain = CalcAttack( samples );
                        if(  FADSRGain>=1 )
                        {
                          FADSRGain =1;
                          FADSRStage = adsrDecay;
                          FDeltaTicks =0;
                        }
                      }
                      break; 
        case adsrDecay:  {
                        FADSRGain = 1- ( 1-S )*CalcDecay( samples );
                        if(  FADSRGain<=S )
                        {
                          FADSRGain =S;
                          FADSRStage = adsrSustain;
                        }
                    }
									 break;

        case adsrSustain:  // make sure ADSRGAin <> 0, because the note will be ended, even if you slide sustain up
                      FADSRGain = MAX( S, 0.001 );
											break;

        case adsrRelease: 
                      if(  ShouldRelease() )
                        FADSRGain = MAX( 0, FADSRGainRelease*( 1-CalcRelease( samples ) ) );
											break;

        }
    }
  return FADSRGain;
}

void TXSynthBlockADSR::Trigger()
{
  FADSRStage = adsrAttack;
  FADSRGain = 0;
  FADSRA =0;
  FADSRD =0;
  FADSRR =0;
  FIsQuickReleasing =false;
  FDeltaTicks =0;
}

void TXSynthBlockADSR::Release(bool Quick)
{
		if (Quick)
				FIsQuickReleasing = true; else
		{
				FADSRStage = adsrRelease;
				FADSRGainRelease = FADSRGain;
				FDeltaTicks = 0;
		}
}

//END
