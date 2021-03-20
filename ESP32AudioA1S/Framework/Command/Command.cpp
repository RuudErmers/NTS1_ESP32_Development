#include "Command.h"

const int SysExStart = 0xF0;
const int SysExEnd = 0xF7;
const int SysExManufacturer = 0x25;
const int MIDI_DEVID = 23;

#define psIdle 0
#define psSysData   2

Command::Command(int Device, int CommSpec0, int CommSpec1, int CommSpec2, int CommSpec3)
{
  this->Device = Device;
  this->CommPort[0] = InitCommunicationPort(0, Device, CommSpec0);
  this->CommPort[1] = InitCommunicationPort(1, Device, CommSpec1);
  this->CommPort[2] = InitCommunicationPort(2, Device, CommSpec2);
  this->CommPort[3] = InitCommunicationPort(3, Device, CommSpec3);
}

void Command::SetSoftwareSerial(int port, Stream *softwareSerial, int Commspec)
{
  this->CommPort[port] = new CommunicationPortSer(this, port, Device, Commspec, softwareSerial);
}

CommunicationPort *Command::InitCommunicationPort(int Port, int Device, int Protocol)
{
  if (Protocol == COMM_DISABLE) return NULL;
  return new CommunicationPortSer(this, Port, Device, Protocol,NULL);
}

void Command::loop()
{
  for (int i = 0; i<4; i++)
    if (CommPort[i] != NULL)
    {
     // Serial.println("SERLOOP"+(String)i);
      CommPort[i]->loop();
    }  
}

void Command::MIDIEvent(int port, int status, int data1, int data2)
{
  if (CommPort[port] != NULL)
    CommPort[port]->MIDIEvent(status, data1, data2);
}

void Command::SysExEvent(int port, SysEx &syx)
{
   if (CommPort[port] == NULL) return; 
   char bytes[32];
   int len = syx.printCC(bytes,32);
   if (len<=0) return;
   for (int i=0;i<len;i++) CommPort[port] ->WriteByte(bytes[i]);
   CommPort[port] ->WriteByte(13);CommPort[port]->WriteByte(10);
   CommPort[port]->WriteComplete();
}

void Command::SysExEvent(int port, int count, byte * sysex)
{
 if (CommPort[port] == NULL) return;  
  for (int i=0;i<count;i++)
    CommPort[port]->WriteByte(sysex[i]);
  CommPort[port]->WriteComplete();  
}

CommunicationPort::CommunicationPort(int Port, int Device, int Protocol)
{
  this->Port = Port;
  this->Device = Device;
  this->Protocol = Protocol;
  setProcessingState(MIDI_CC);
}

int sysexLength = 0;

void CommunicationPort::setProcessingState(int state)
{
  processingState = state;
  if (state == psSysData) dataP = 0;
  else dataP = 1; // [0] for status byte
}

void CommunicationPort::loop()
{
  if (Protocol & COMM_INPUTMIDI)
    while (true)
    {
    //  Serial.println("Getting byte...");
      int b = ReadByte();
      if (b==-1) return;
      if (processingState != psSysData) sysexLength = 0;
//      Serial.println((String)b);
      if (b <= 0xF7)
      {
        if ((b & 0x80) != 0)
        {
          if (b == SysExStart)
          {
            setProcessingState(psSysData);
            sysexLength = 1;
            setSysData(b);
          }
          else if (b == SysExEnd)
          {
            if (processingState == psSysData)
            {
              sysexLength++;
              setSysData(b);
              doSysExCommand();
            }
            setProcessingState(psIdle);
          }
          else setProcessingState(b);
        }
        else if (processingState == psSysData)
        {
          setSysData(b);
          sysexLength++;
        }
        else if (processingState != psIdle) setData(b);
      }
      //    if (sysexLength>0) Serial.println("SYX:" + (String)sysexLength);
    }
  else
  {
    while (true)
    {
      int b = ReadByte();
      if (b==-1) return;
//      Serial.println("Serial:" +(String)Port + (String)b);
      if (b == 13)
      {
        serialEvent(dataBuffer, dataP);
        dataP = 0;
      }
      else if ((b>20) && (b<127) && (dataP<DATABUFFERSIZE - 1))
        dataBuffer[dataP++] = b;
    }
  }
}

void CommunicationPort::replyIdentity()
{
  WriteByte(SysExStart);
  WriteByte(SysExManufacturer);
  WriteByte(0);
  WriteByte(MIDI_DEVID);
  WriteByte(1);
  WriteByte(Device);
  WriteByte(SysExEnd);
  WriteComplete();
}

void CommunicationPort::doSysExCommand()
{
#define MAN_CRUMAR 0x25
  if (dataP == 0) return;
  SysEx sysex(dataBuffer, dataP);
  if (sysex.manufacturer() == MAN_CRUMAR)
  {
    if (sysex.status() == MIDI_DEVID)
      replyIdentity();
    else
      onSysExEvent(Port, sysex);
  }    
}

void CommunicationPort::setSysData(unsigned int b)
{
  if (dataP < DATABUFFERSIZE)
    dataBuffer[dataP++] = b;
  else
    setProcessingState(psIdle);
}

void CommunicationPort::setData(unsigned int b)
{
  int dataBytes;
  if (dataP < DATABUFFERSIZE)  
    dataBuffer[dataP++] = b;
  //  Serial.println("setData:" + (String)b+" " + (String)dataP+" " + (String)processingState);
  switch (processingState & 0xF0)
  {
  case MIDI_PITCHWHEEL:
  case MIDI_NOTEON:
  case MIDI_NOTEOFF:
  case MIDI_CC: dataBytes = 2; break;
  case MIDI_PRG: dataBytes = 1;
  case MIDI_AFTERTOUCH: dataBytes = 1; break;
  default: return;
  }
  if (dataP == dataBytes + 1)
  {
    dataBuffer[0] = processingState;
    //    Serial.println("setData:serialEvent------->");
    serialEvent(dataBuffer, dataP);
// from 09092019 I REQUIRE a repeat of the statusbyte
    processingState = psIdle; 
//    dataP = 1;
  }
}

int fromHex(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
}

int fromHex2(char *p)
{
	return 16 * fromHex(p[0]) + fromHex(p[1]);
}

bool fromCC(byte *bytebuffer, int &midi, int &data1, int &data2, bool &second,int &data3, int &data4)
// valid strings: CCXXYY, CMNXXYY, DDXXXXYYYY, DMXXXXYYYY
// this code has ao. this error:  DDCC00110011 will be seen as CC00110011 , therefor send data in lower case: DDcc00110011 
{
	midi = 0;
	char *p = NULL, *buffer = (char *)bytebuffer;
//	Serial.print("WIFI: ");
//	Serial.println(buffer);
	if (p == NULL) {
		p = strstr(buffer, "CC"); if (p != NULL) {
			second = false;
		}
	}
	if (p == NULL) {
		p = strstr(buffer, "CM"); if (p != NULL) {
			second = false; midi = fromHex(p[2]); p++;
		}
	}
	if (p == NULL) {
		p = strstr(buffer, "DD"); if (p != NULL) {
			second = true;
		}
	}
	if (p == NULL) {
		p = strstr(buffer, "DM"); if (p != NULL) {
			second = true; midi = fromHex(p[2]); p++;
		}
	}
	if (p == NULL) return false;
	data1 = fromHex2(p+2);
	data2 = fromHex2(p+4);
	data3 = fromHex2(p+6);
	data4 = fromHex2(p+8);
	if ((data1 < 0) || (data1>127)) return false;
	if ((data2 < 0) || (data2>127)) return false;
	if ((midi < 0) || (midi > 15)) return false;
	if (!second) return true;
	if ((data3 < 0) || (data3>127)) return false;
	if ((data4 < 0) || (data4>127)) return false;
	return true;
}

void CommunicationPort::serialEvent(byte *bytes, int length)
{
  //if (Port == 2) 
  //  Serial.println("Event!" + (String)length + " " + (String)bytes[0]+" " + (String)bytes[1] + " " +(String)bytes[2]);
  if (Protocol & COMM_INPUTMIDI)
  {
    switch (bytes[0] & 0xF0)
    {
    case MIDI_PRG:
    case MIDI_AFTERTOUCH: bytes[2] = 0; // only 1 databyte
    case MIDI_PITCHWHEEL:
    case MIDI_CC:
    case MIDI_NOTEOFF:
    case MIDI_NOTEON: onMIDIEvent(bytes[0], bytes[1], bytes[2]);
    
      // For measuring throughput
      //   if (bytes[1] == 13)
      //              Parent->MIDIEvent(Port, MIDI_CC, 90, bytes[2]);
      break;
    default: return;
    }

  }
  else
  {
	  int midi, data1, data2, data3, data4;
	  bool second;
	  bytes[length] = 0;
	  if (fromCC(bytes, midi, data1, data2, second, data3, data4))
	  {
//		  Serial.println("MIDI CC:" + (String)midi + " " + (String)data1 + " " + (String)data2);
		  onMIDIEvent(MIDI_CC + midi, data1, data2);
		  if (second)
		  {
//			  Serial.println("MIDI CC:" + (String)midi + " " + (String)data3 + " " + (String)data4);
			  onMIDIEvent(MIDI_CC + midi, data3, data4);
		  }
	  }
  }
}

const char hex[] = "0123456789ABCDEF";

String toHex(int cntrl)
{
  return (String)hex[cntrl / 16] + (String)hex[cntrl % 16];
}


String toMIDI(int status, int data1, int data2)
{
  String result = "XX";
  switch (status & 0xF0)
  {
  case MIDI_CC: result = "CC"; break;
  case MIDI_NOTEON: result = "KD"; break;
  case MIDI_NOTEOFF: result = "KU"; break;
  case MIDI_PITCHWHEEL: result = "PW"; break;
  case MIDI_AFTERTOUCH: result = "AF"; break;
  case MIDI_PRG:result = "PG"; break;
  }
  return result + toHex(data1) + toHex(data2);
}


void CommunicationPort::MIDIEvent(int status, int data1, int data2)
{
  //  Serial.println("Try CCEvent over port=" + (String)Port);
  if (Protocol & COMM_OUTPUTMIDI)
  {
    // Serial.println("Midi Out Over "+(String)Port+" Status=" + (String)status + " Data1=" + (String)data1 + " Data2=" + (String)data2);
    if (((status & 0xF0) == MIDI_CC) && (data1 >= 128 || data2 >= 128))
    {
      WriteByte(MIDI_CC);
      WriteByte(99); WriteByte(data1 / 0x80);
      WriteByte(98); WriteByte(data1 % 0x80);
      WriteByte(6);  WriteByte(data2 / 0x80);
      WriteByte(38); WriteByte(data2 % 0x80);
    }
    else
    {
      WriteByte(status);
      WriteByte(data1);
      if (((status & 0xF0) != MIDI_AFTERTOUCH) && ((status & 0xF0) != MIDI_PRG))
        WriteByte(data2);
    }
  }
  else 
  {
    String s =toMIDI(status, data1, data2);
    int len= s.length();
    for (int i=0;i<len;i++) WriteByte(s.c_str()[i]);
    WriteByte(13);WriteByte(10);

  }
  WriteComplete();
}


bool SysEx::printCC(char *dataS, int len)  // len must be at least 32
{
  sprintf(dataS,"SX%02X%04X%04X",status(),data1(),data2());
  int buflen = length - 9;
  int slen = 12;
  if (slen+buflen>=len-2) return false;
  for (int i=0;i<buflen;i++) dataS[slen++]=buffer[8+i];
  dataS[slen]=0;
}


void ShowMem()
{
/*
#ifndef __SAM3X8E__
  uint8_t *m = (uint8_t *)malloc(4);
  Serial.println("SP = " + (String)(int)SP + " HP = " + (String)(int)m);
  free(m);
#endif  */
}

CommunicationPortSer::CommunicationPortSer(Command *Parent,int Port, int Device, int Protocol, Stream *serial) : CommunicationPort(Port,Device,Protocol), Parent(Parent), pSerial(serial)
{
  if (pSerial == NULL)
    switch (Port)
    {
      case 0: pSerial = &Serial; break;
#ifdef HAVE_HWSERIAL1
      case 1: pSerial = &Serial1; break;
      case 2: pSerial = &Serial2; break;
      case 3: pSerial = &Serial3; break;          
#else
      case 1: pSerial = &Serial; break;
      case 2: pSerial = &Serial; break;
      case 3: pSerial = &Serial; break;
#endif
    }
}
