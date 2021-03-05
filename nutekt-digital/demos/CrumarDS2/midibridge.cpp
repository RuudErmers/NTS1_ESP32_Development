#include "userosc.h"
#include "myos.h"
enum { mmNone, mmData1,mmData2};
// Er is een Document! KorgSimCC

void MidiMessageReceived(int s,int t,int abcd, int efghi, int jklmn)
{
  int status = 2*t+s;
  int data1= (abcd << 3) + (efghi>>2);
  int data2=(((efghi) & 3)  << 5) + jklmn;
  switch (status)
  {
     case 0: /* CC */ 
                      OSC_SetParameter(data1,data2);
                      break;
		 case 1: /* PRG */
				              break;
		 case 2: /* CHNL */ 
				              break;
		 case 3: /* PW */ 
				              break;
  }
} 

void checkMidiMode(uint16_t index, uint16_t value)
{
// Ja er komt wat nbinnen...  OSC_SetParameter(53,value / 8);
		static int MidiMode, s,t,abcd,efghi,jklmn;
		if (value % 8 != 0) MidiMode = mmNone;
		else
		{
				int cc = value / 8;
				switch (MidiMode)
				{
          case mmNone: switch (cc & 3) 
                        { case 0: case 3: t = ((cc&3)==3); MidiMode = mmData1; s = (cc >> 6) & 1; abcd= (cc>>2) & 0xF; break;
                         default: MidiMode = mmNone;
                        }
												break;
          case mmData1: if ((cc & 3) !=1) MidiMode = mmNone;
                       else { efghi = (cc>>2) & 0x1F; MidiMode = mmData2; }
                       break;
					case mmData2: if ((cc & 3) != 2) MidiMode = mmNone;
												else 
                        { 
                          jklmn = (cc>>2) & 0x1F; 
												  MidiMessageReceived(s,t,abcd,efghi,jklmn);
													MidiMode = mmNone;
                        }
												break;
        }
    }
}

void OSC_PARAM(uint16_t index,uint16_t value)
{
		if (index== k_user_osc_param_shiftshape)
				checkMidiMode(index, value);
    else
		  OSC_PARAM_NEW(index, value);
}


