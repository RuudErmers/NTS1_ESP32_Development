#include "UserOsc.h"
#include "UAudioTypes.h"

static int16_t sample[2];

void XUserOsc::loop()
{
  
  bool ok;
  static int tot=0,laps=0,verytot=0,proccalls;
  static int maxSamplesDone;  
  int samplesDone=0;
  do 
  {
      if (BufferP>=32) { Process();proccalls++; }

      ok = output->ConsumeSample(sample);
      if (ok) 
      {
        int32_t nsample =  Buffer[BufferP++], sig;;
//        sample[0]=nsample/0x100000000; 
//        sample[1]=nsample/0x100000000;
        if (nsample<0)  { sig = -1; nsample=-nsample; } else sig=1;
        sample[0]=sig*(nsample>>16); 
        sample[1]=sample[0];
/*		
#define N 20000		
		if ( sample[0]>=N) sample[0]=N;
		if ( sample[1]>=N) sample[1]=N;
		if ( sample[0]<=-N) sample[0]=-N;
		if ( sample[1]<=-N) sample[1]=-N;
*/
        tot++;
        verytot++;
        if (tot>40000)
        { 
          Serial.printf("weer een seconde: LAPS: %d SAMPLES: %d PROCALLS:%d MAXSAMPLES %d\r\n",laps,verytot,proccalls,maxSamplesDone);
		  laps=0;
		  maxSamplesDone=0;
          tot=0;
        }
      }  
  }
  while (ok  &&(samplesDone++<500));
  if ( samplesDone>maxSamplesDone) maxSamplesDone=samplesDone;
  laps++;
//  Serial.printf("L OUT %d",verytot);

}

static int pitch = 0;
user_osc_param_t PARAMS;

void XUserOsc::Process()
{
  OSC_CYCLE(&PARAMS, Buffer, 32);
  BufferP=0;
}

XUserOsc::XUserOsc() 
{ 
	output = new AudioOutputI2S();
    output->begin(false);	
	output->SetGain(0.2);
		OSC_INIT(0, 0);
		// TODO: Fill PARAMS
		/** Value of LFO implicitely applied to shape parameter */
		PARAMS.shape_lfo = 0;
		/** Current pitch. high byte: note number, low byte: fine (0-255) */
		PARAMS.pitch = 0;
		/** Current cutoff value (0x0000-0x1fff) */
		PARAMS.cutoff = 0;
		/** Current resonance value (0x0000-0x1fff) */
		PARAMS.resonance = 0;
		for (int i = 0; i<3; i++) PARAMS.reserved0[i] = 0;  
    BufferP=32; // force read 
}


float osc_notehzf(uint8_t note) {
		return 8.18*pow(13289 * 1.0 / 8.18, note / 128.0);
}

float fsel(const float a, const float b, const float c) {
		return (a >= 0) ? b : c;
}

float clampmaxfsel(const float x, const float max)
{
		return fsel(x - max, max, x);
}

float clipmaxf(float x, float m)
{
		return (((x) >= m) ? m : (x));
}

float linintf(const float fr, const float x0, const float x1)
{
		return x0 + fr * (x1 - x0);
}


float osc_w0f_for_note(uint8_t note, uint8_t mod) {
		const float f0 = osc_notehzf(note);
		const float f1 = osc_notehzf(note + 1);

		const float f = clipmaxf(linintf(mod * k_note_mod_fscale, f0, f1), k_note_max_hz);

		return f * k_samplerate_recipf;
}

float clipminmaxf(const float min, const float x, const float max)
{
		return (((x) >= max) ? max : ((x) <= min) ? min : (x));
}
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

void checkMidiMode(uint16_t &index, uint16_t &value)
{
  //DEBUG("Check Midi Mode %d\n",value);
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



#define MIDI_CC  0xB0
#define MIDI_NOTEON  0x90
#define MIDI_NOTEOFF  0x80

void ProcessMidi(CMidiEvent &midievent)
{
		int data1 = midievent.data1;
		int data2 = midievent.data2;
		if (midievent.isNoteOn())
		{
				// TODO set Param 
				if ((data1 != pitch) && (pitch>0)) OSC_NOTEOFF(&PARAMS);
				PARAMS.pitch = data1 << 8;
				pitch = data1;
				OSC_NOTEON(&PARAMS);
		}
		if (midievent.isNoteOff())
		{
				if (pitch == data1)
				{
						pitch = 0;
						OSC_NOTEOFF(&PARAMS);
				}
		}
		if (midievent.isCC())
		{
				if (data1 == 55) OSC_PARAM(k_user_osc_param_shiftshape, data2 * 8);
		}		
		//else hoeft niet meer....		 
        //   setParameter(data1,round(data2*100.0/127));		  


}

#include "Command.h"

class MyCommand : public Command
{
public:
  MyCommand(int Device) : Command(Device, COMM_INPUTMIDI | COMM_OUTPUTMIDI) {} //,COMM_INPUTMIDI | COMM_OUTPUTMIDI) {}
  void onMIDIEvent(int port, int status, int data1, int data2) override;
};

MyCommand *command;

XUserOsc *osc;

void fwSetup()
{
  Serial.begin(4L*9600);   // 31250 is not possible
  command = new MyCommand(0);  
  Serial.println("Starting....");
  osc = new XUserOsc();   
}


void fwLoop()
{
  command->loop();
  osc->loop();  
}

void MyCommand::onMIDIEvent(int port, int status, int data1, int data2)
{
  CMidiEvent midiEvent(status,data1,data2);
  Serial.printf("Event %s",midiEvent.ToString());
  if (midiEvent.isNoteOff() && (midiEvent.data1==0))
     ESP.restart();
  ProcessMidi(midiEvent);
}
