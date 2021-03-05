#include "userosc.h"
#include "UMidiPorts.h"
#include "myOS.h"

/* template...
void OSC_INIT(uint32_t platform, uint32_t api)
{
(void)platform;
(void)api;
}

void OSC_CYCLE(const user_osc_param_t * const params,		int32_t *yn, const uint32_t frames)
{
//		updatePitch(osc_w0f_for_note((params->pitch) >> 8, params->pitch & 0xFF));
// s.lfo = q31_to_f32(params->shape_lfo);
// shape_lfo is gewoon de stand van de lfo op dat moment/ laten we 'm hier ignoren...
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
switch (index) {
case k_user_osc_param_id1:
case k_user_osc_param_id2:
case k_user_osc_param_id3:
case k_user_osc_param_id4:
case k_user_osc_param_id5:
case k_user_osc_param_id6:
break;
case k_user_osc_param_shape:
// 10bit parameter
//p.shape = param_val_to_f32(value);
break;

case k_user_osc_param_shiftshape:
// 10bit parameter
//p.shiftshape = 1.f + param_val_to_f32(value);
break;
default:
break;
}
}
*/

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
                      DEBUG("Set Parameter: %d %d\n",data1,data2); 
                      OSC_setParameter(data1,data2);
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
  DEBUG("Check Midi Mode %d\n",value);
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

/////////////////////////////////////////////////////////////////////////////////

user_osc_param_t PARAMS;

#define MIDI_CC  0xB0
#define MIDI_NOTEON  0x90
#define MIDI_NOTEOFF  0x80

static int pitch = 0;
void MidiInput(CMidiDevice *midiDevice, CMidiEvent midievent)
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
				if (data1 == 55) 
               OSC_PARAM(k_user_osc_param_shiftshape, data2 * 8);
}

CMidiDevice *FMidiDevice;

//extern "C" 
void ModelCreate()
{
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
		FMidiDevice = new CMidiDevice();
		FMidiDevice->Open("Super52 Out-01-16");
		FMidiDevice->OnMidiInput = MidiInput;
}

//#define SetOnMidiInput(func) [this](CMidiDevice *port,CMidiEvent m) { (func)(port,m); };

#define BUFFERSIZE 32

//extern "C" 
void ModelProcess(short *buffer, int length)
{
		static int32_t tBuffer[BUFFERSIZE];
		int pos = 0;
		while (length>0)
		{
				int samples = min(BUFFERSIZE, length);
				if (pitch>0)
						OSC_CYCLE(&PARAMS, tBuffer, samples);
				// and copy ...
				for (int i = 0; i<samples; i++)
				{

						if (pitch>0)
								*buffer++ = tBuffer[i] >> 16;
						else
								*buffer++ = 0;

				}
				length -= samples;
		}
}







/* template...
void OSC_INIT(uint32_t platform, uint32_t api)
{
(void)platform;
(void)api;
}

void OSC_CYCLE(const user_osc_param_t * const params,		int32_t *yn, const uint32_t frames)
{
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
switch (index) {
case k_user_osc_param_id1:
case k_user_osc_param_id2:
case k_user_osc_param_id3:
case k_user_osc_param_id4:
case k_user_osc_param_id5:
case k_user_osc_param_id6:
break;
case k_user_osc_param_shape:
// 10bit parameter
//p.shape = param_val_to_f32(value);
break;

case k_user_osc_param_shiftshape:
// 10bit parameter
//p.shiftshape = 1.f + param_val_to_f32(value);
break;
default:
break;
}
}
*/