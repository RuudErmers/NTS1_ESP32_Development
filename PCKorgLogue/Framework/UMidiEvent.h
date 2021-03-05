#ifndef _UCMidiEvent__H
#define _UCMidiEvent__H
#include <stdio.h>
#include <stdlib.h>

#define MIDI_CC  0xB0
#define MIDI_NOTEON  0x90
#define MIDI_NOTEOFF  0x80
#define MIDI_PRG  0xC0
#define MIDI_PW 0xE0
#define MIDI_PITCHWHEEL  0xE0
#define MIDI_AFTERTOUCH  0xD0

class CMidiEvent {
public:
  int status,data1,data2;    
  CMidiEvent(int status,int data1,int data2) : status(status),data1(data1),data2(data2) {}
  bool isNoteOn() { return (((status& 0xF0) ==MIDI_NOTEON) && (data2>0));}
  bool isNoteOff() { return ((((status & 0xF0)==MIDI_NOTEON) && (data2==0)) || ((status & 0xF0)==MIDI_NOTEOFF));}
  bool isCC() { return  ((status & 0xF0) ==MIDI_CC); }
  char *ToString() {  static char fmt[24]; sprintf(fmt,"%d %d %d",status,data1,data2); return fmt; } 
};
#endif
