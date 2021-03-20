#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <Arduino.h>


const int DATABUFFERSIZE = 32;
const int COMM_INPUTMIDI = 1;
const int COMM_OUTPUTMIDI = 2;
const int COMM_DISABLE = -1;

#define MIDI_CC  0xB0
#define MIDI_NOTEON  0x90
#define MIDI_NOTEOFF  0x80
#define MIDI_PRG  0xC0
#define MIDI_PITCHWHEEL  0xE0
#define MIDI_AFTERTOUCH  0xD0

class Command;

class SysEx {
public:
  SysEx(byte *buffer, int length) 
  { this->buffer=buffer;
    this->length = length;
    buffer[length - 1] = 0; // 0xFE
    while (this->length<=7) buffer[this->length++]=0;   
  }
  int data1() { return buffer[4] + (buffer[6]<<7); }
  int data2() { return buffer[5] + (buffer[7] << 7); }
  int manufacturer() { return buffer[1]; }
  int midichannel() { return buffer[2]; }
  int status() { return buffer[3]; }
  char *dataS() { buffer[length - 1] = 0; if (length<=8) buffer[8]=0; return (char *)buffer + 8; }
  bool printCC(char *dataS, int len);
private:
  byte *buffer;
  byte length;

};
class CommunicationPort {
public:
  void loop();
  CommunicationPort(int Port, int Device, int Protocol);
  void MIDIEvent(int status, int data1, int data2);
  virtual int ReadByte() {return -1;}
  virtual void WriteByte(int b) {}
  virtual void WriteComplete() {}
  virtual void onSysExEvent(int port, SysEx &sysex) {}  
//  Stream *serPort() { return pSerial; }
protected:
  void serialEvent(byte *bytes, int length);
  virtual void onMIDIEvent(int status, int data1, int data2) {}
  byte Port,Protocol,Device;
private:
  void replyIdentity();
  void setProcessingState(int state);
  void doSysExCommand();
  void setData(unsigned int b);
  void setSysData(unsigned int b);
  byte dataBuffer[DATABUFFERSIZE];
  byte dataP;
  byte processingState;
};

class Command {
public:
  Command(int Device, int CommSpec0 = COMM_DISABLE, int CommSpec1 = COMM_DISABLE, int CommSpec2 = COMM_DISABLE, int CommSpec3 = COMM_DISABLE);
  void loop();
  virtual void onMIDIEvent(int port, int status, int data1, int data2) {}
  virtual void onSysExEvent(int port, SysEx &sysex) {}
  void MIDIEvent(int port,int status, int data1, int data2);
  void SysExEvent(int port,SysEx &syx);
  void SysExEvent(int port, int count, byte *sysex);
  void SetSoftwareSerial(int port, Stream *softwareSerial,int Commspec);
protected:  
private:
  int Device;
  CommunicationPort *CommPort[4];
  CommunicationPort *InitCommunicationPort(int Id, int Device, int Protocol);
};

class CommunicationPortSer : public CommunicationPort
{
private:  
  Command *Parent;
  Stream *pSerial;
public:  
  CommunicationPortSer(Command *Parent,int Port, int Device, int Protocol, Stream *serial);
  virtual int ReadByte() { return (pSerial->available()==0) ? -1 : pSerial->read(); }
  virtual void WriteByte(int b) { pSerial->write(b); }
  virtual void onSysExEvent(int port, SysEx &sysex) { Parent->onSysExEvent(port,sysex);}  
  virtual void onMIDIEvent(int status, int data1, int data2) { Parent->onMIDIEvent(Port, status, data1, data2); }
};

void ShowMem();

#endif
  
