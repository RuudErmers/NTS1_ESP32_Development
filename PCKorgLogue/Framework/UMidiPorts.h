#ifndef __UMIDIPORTS_H
#define __UMIDIPORTS_H

// Platform-dependent sleep routines.
#if defined(WIN32)
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

#include "UMidiEvent.h"
#include "RtMidi.h"
#include <functional>

/*

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF  0x80
#define MIDI_POLYPRESSURE  0xA0
#define MIDI_CC  0xB0
#define MIDI_PRG  0xC0
#define MIDI_PW 0xE0

/*
typedef struct {
		int port, midichannel, status, data1, data2;
		bool isNoteOn() { return ((status == MIDI_NOTE_ON) && (data2>0));}
} CMidiEvent;
*/


typedef std::function<void(CMidiEvent)> TMidiOutput;

class CMidiOutPort : public RtMidiOut
{
private:
		bool FConnected;
public:
		CMidiOutPort() : RtMidiOut() {}
		void Open(std::string portname);
		void WriteMidi(CMidiEvent midievent);
};

class CMidiDevice;
class CMidiInPort;
typedef std::function<void(CMidiDevice*,CMidiEvent)> TonInputMidi;
typedef std::function<void(CMidiInPort*, CMidiEvent)> TonInputMidiPort;


class CMidiInPort : public RtMidiIn
{
private:
		bool FConnected;
public:
		TonInputMidiPort OnMidiInput;
		CMidiInPort() : RtMidiIn(), OnMidiInput(NULL),FConnected(false) {}
		void CBMidiInput(CMidiEvent midievent);
		void Open(std::string portname);
};

#define SetOnMidiOutput(func) [this](CMidiEvent m) { (func)(m); };
#define SetOnMidiInput(func) [this](CMidiDevice *port,CMidiEvent m) { (func)(port,m); };

CMidiEvent MidiEvent(int midichannel, int status, int data1, int data2);

class CMidiDevice
{
private:
		CMidiOutPort *midiOutport;
		CMidiInPort *midiInport;
public:
		TonInputMidi OnMidiInput;  // assign with e.g. midiDevice->OnMidiInput = SetOnMidiInput(MidiInput);
		CMidiDevice();
		void Open(std::string portName);
		void WriteMidi(CMidiEvent midievent) { midiOutport->WriteMidi(midievent); }
};

#endif
