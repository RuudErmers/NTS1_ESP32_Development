#include "blackstomp.h"
#include "UAudioTypes.h"


#include "userosc.h"
#include "math.h"
user_osc_param_t PARAMS;

// The Audio Board has three parts:
// 1. Input from Line 1                         #define DO_READ_LINE_IN    true
// 2. Input from Synth (OSC_CYCLE)              #define  DO_OSC_CYCLE      true
// 3. Effects Session with inputs 1 and 2       #define DO_EFFECTS         true
//  

#define DO_READ_LINE_IN  false
#define DO_OSC_CYCLE    true
#define DO_EFFECTS   false
#define EFFECTS_ON_CORE0

#define isEffectProcessor (DO_EFFECTS and DO_READ_LINE_IN and (not DO_OSC_CYCLE))
#if DO_EFFECTS == true
#define PROCESS_BUF_SIZE 256
extern void processEffects(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
extern void initEffects();
CAudioBuffer EffectsBufferIn(PROCESS_BUF_SIZE),EffectsBufferOut(PROCESS_BUF_SIZE);
#endif


void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount) // sampleCount is allways 32
{
//   
  
  static int32_t buf[128];
  if (not isEffectProcessor)
  {
    if (DO_OSC_CYCLE)
      OSC_CYCLE(&PARAMS, buf, sampleCount);
    for (int i = 0; i < sampleCount; i++)
    {
      float vl = 0;
      float vr = 0;
      if (DO_READ_LINE_IN)
      {
        vl = inLeft[i];
        vr = inLeft[i];
      }
      if (DO_OSC_CYCLE)
      {
        float v = buf[i] * 1.0 / 0x7FFFFFF0;
        vl += v;
        vr += v;
      }
      if (DO_EFFECTS)
      {
        inLeft[i] = vl;
        inRight[i] = vr;
      }
      else
      {
        outLeft[i] = vl;
        outRight[i] = vr;
      }
    }
  }     
#if DO_EFFECTS==true

#ifdef EFFECTS_ON_CORE0
   for (int i=0;i<sampleCount;i++) EffectsBufferIn.WriteSampleSync(TSample(inLeft[i],inRight[i]));
   for (int i=0;i<sampleCount;i++) 
   {
     TSample sample;
     EffectsBufferOut.ReadSampleSync(sample);
     outLeft[i]=sample.l;
     outRight[i]=sample.r;
   }
#else
     processEffects(ProcessinLeft, ProcessinRight, ProcessoutLeft, ProcessoutRight,len);
#endif   
#endif     
} 
/*  
  for(int i=0;i<sampleCount;i++)
  {
    outLeft[i] =  buf[i]*1.0/0x7FFFFFF0;
    outRight[i] =  outLeft[i];
  } 
  /*
  for(int i=0;i<sampleCount;i++)
  {
    outLeft[i] =  inLeft[i];
    outRight[i] =  inRight[i];
  } 
  */

float osc_notehzf(uint8_t note) {
  static float val[128];
  if (note>=128) return 0;   
  if (val[note]<0.01) val[note]=8.18*pow(13289 * 1.0 / 8.18, note / 128.0);
	return val[note];
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

static int pitch = 0;

void osc_note_off()
{
  	pitch = 0;
	OSC_NOTEOFF(&PARAMS);
}	

void osc_note_on(uint16_t newPitch)
{
	if ((newPitch != pitch) && (pitch>0)) osc_note_off();
    PARAMS.pitch = newPitch << 8;  
    pitch = newPitch;
    OSC_NOTEON(&PARAMS);
}

void ProcessMidi(CMidiEvent &midievent)
{

		int data1 = midievent.data1;
		int data2 = midievent.data2;
      bool korgmode = (midievent.status&0xF)>=12;
	  if (midievent.isNoteOn())
		  {
          if (korgmode)
			osc_note_on(data1);
          else 
			OSC_DoNoteEvent(data1,data2);
		  }
	if (midievent.isNoteOff())
	 {
        if (korgmode)
        {
		  if (pitch == data1) osc_note_off();
        }
		else
   		 OSC_DoNoteEvent(data1, data2);
     }
	if (midievent.isCC())
      {
					if (korgmode)
					{
							if (data1 == 55)
									OSC_PARAM(k_user_osc_param_shiftshape, data2 * 8);
          }
          else 
               OSC_SetParameter(data1,data2);
		  }
}

#include "Command.h"

class MyCommand : public Command
{
public:
  MyCommand(int Device) : Command(Device, COMM_INPUTMIDI | COMM_OUTPUTMIDI) {} //,COMM_INPUTMIDI | COMM_OUTPUTMIDI) {}
  void onMIDIEvent(int port, int status, int data1, int data2) override;
};

MyCommand *command;

void logMemory() {
    Serial.printf("Used PSRAM: %ld PSRam Left: %ld\r\n", ESP.getPsramSize() - ESP.getFreePsram(),ESP.getFreePsram());
}

void getRam() {
  logMemory();
  byte* psdRamBuffer = (byte*)ps_malloc(500000);
  logMemory();
  free(psdRamBuffer);
  logMemory();
}

static int keypin[6]= { 36,13,19,23,18,5}; 
void checkKeys()
{
  static bool keystate[6];
  static unsigned long lastMillis;
  if (millis()>=lastMillis+50)
  { lastMillis=millis();
    for (int i=0;i<6;i++)
    { 
	  bool state = !digitalRead(keypin[i]);
	  if (state!=keystate[i])
	  {
		  keystate[i]=state;
		  // and do something with <i,state> 
		  Serial.println("Doing Key"+(String)i+" "+(String)(!!state));
		  if (state) osc_note_on(60+2*i);
		  else       osc_note_off();
		  if (i==5) getRam();
	  }  
	}
  }
}	  

void MyCommand::onMIDIEvent(int port, int status, int data1, int data2)
{
  CMidiEvent midiEvent(status,data1,data2);
  Serial.printf("Event %s\r\n",midiEvent.ToString());
  if (midiEvent.isNoteOff() && (midiEvent.data1==0))
     ESP.restart();
  ProcessMidi(midiEvent);
}

//Arduino core setup

void fwSetup()
{
  //setting up the effect module
  Serial.begin(38400); 
#if DO_EFFECTS == true  
  initEffects();
  for (int i=0;i<PROCESS_BUF_SIZE;i++) EffectsBufferIn.WriteSample(0); 
#endif 
  OSC_INIT(0,0);
  blackstompSetup(DO_READ_LINE_IN);
  setOutVol(30);
  //run system monitor at 38400 baud rat, at 2000 ms update period
  //don't call this function when MIDI is implemented
  //try lowering the baudrate if audible noise is introduced on some boards
  runSystemMonitor(10000);
	//Enable amplifier
#define GPIO_PA_EN 21
//	pinMode(GPIO_PA_EN, OUTPUT);
//	digitalWrite(GPIO_PA_EN, HIGH);

	// Configure keys on ESP32 Audio Kit board
	for (int i=0;i<6;i++) pinMode(keypin[i], INPUT_PULLUP);
  command = new MyCommand(0);  
  Serial.println("Starting....");

 
}

#if DO_EFFECTS == true 
#ifdef EFFECTS_ON_CORE0

 static float ProcessinLeft[PROCESS_BUF_SIZE],ProcessinRight[PROCESS_BUF_SIZE],ProcessoutLeft[PROCESS_BUF_SIZE],ProcessoutRight[PROCESS_BUF_SIZE];

void checkProcess()
{
  static unsigned long lastMillis;
  //if (millis()>lastMillis+1)
  {
     lastMillis=millis();
     int len = min(EffectsBufferIn.Size(),PROCESS_BUF_SIZE-EffectsBufferOut.Size());
     if (len>0)
     {
     for (int i=0;i<len;i++)
     {
       TSample sample;
       EffectsBufferIn.ReadSample(sample);
       ProcessinLeft[i]=sample.l;
       ProcessinRight[i]=sample.r;
     }
     processEffects(ProcessinLeft, ProcessinRight, ProcessoutLeft, ProcessoutRight,len);
     for (int i=0;i<len;i++)
            EffectsBufferOut.WriteSample(TSample(ProcessoutLeft[i],ProcessoutRight[i]));
     }
  }
   
}
#endif     
#endif     


void fwLoop()
{
  command->loop();
  checkKeys();
#if DO_EFFECTS == true 
#ifdef EFFECTS_ON_CORE0
  checkProcess();
#endif  
#endif     
}	


          
