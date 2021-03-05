#include "UMidiPorts.h"

void CMidiOutPort::Open(std::string portname)
{
		std::cout << " Probing: " << portname << std::endl;
		FConnected = false;
		for (int i = 0; i<getPortCount(); i++)
		{
				std::cout << " Trying Out Port: " << getPortName(i) << std::endl;
				if (getPortName(i).find(portname)==0)
				{
						std::cout << " Open Out Port: " << i << std::endl;
						openPort(i);
						FConnected = true;
				}
		}
}

void mycallback(float deltatime, std::vector< unsigned char > *message, void *userData)
{
		//unsigned int nBytes = message->size();
		int byte0 = message->at(0);
		int byte1 = (message->size()>1 ? message->at(1) : 0);
		int byte2 = (message->size()>2 ? message->at(2) : 0);
		// daz ganze midi protocol, but for now only CC, PRG and note events..
		switch (byte0 & 0xF0)
		{
		case MIDI_NOTEON: case MIDI_NOTEOFF: case MIDI_CC: case MIDI_PRG: case MIDI_PW: break;
		default: return; // ignore;
		}
		CMidiInPort *midiInport = (CMidiInPort *)userData;
		CMidiEvent m(byte0, byte1, byte2);
		midiInport->CBMidiInput(CMidiEvent(m));
}

void CMidiInPort::CBMidiInput(CMidiEvent midievent)
{
  if (OnMidiInput !=NULL)
			OnMidiInput(this,midievent);
}


void CMidiInPort::Open(std::string portname)
{
		std::cout << " Probing: " << portname << std::endl;
		FConnected = false;
		for (int i = 0; i<getPortCount(); i++)
		{
				std::cout << " Trying Out Port: " << getPortName(i) << std::endl;
				if (getPortName(i).find(portname)==0)
				{
						std::cout << " Open In Port: " << i << std::endl;
						openPort(i);
						setCallback(&mycallback, this);
						FConnected = true;

				}

		}
}

void CMidiOutPort::WriteMidi(CMidiEvent midievent)
{
	if (!FConnected) return;
	std::vector<unsigned char> message;
	message.push_back(midievent.status);
	message.push_back(midievent.data1);
	if (midievent.status != MIDI_PRG)
	  message.push_back(midievent.data2);
	sendMessage(&message);
}

CMidiDevice::CMidiDevice()
{
		midiInport  = new CMidiInPort();
		midiOutport = new CMidiOutPort();
}

void CMidiDevice::Open(std::string portName)
{
		midiInport->Open(portName);
		midiInport->OnMidiInput = [this](CMidiInPort *port, CMidiEvent m) 
                            { if (OnMidiInput!=NULL) OnMidiInput(this,m); };
		midiOutport->Open(portName);
}
