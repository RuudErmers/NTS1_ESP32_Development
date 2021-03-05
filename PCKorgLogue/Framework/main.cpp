/*
	BASS simple synth
	Copyright (c) 2001-2017 Un4seen Developments Ltd.
*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "bass.h"
extern void ModelCreate(char *midiinput);
extern void ModelProcess(short *buffer, int samples); 

BASS_INFO info;
HSTREAM stream; // the stream

// display error messages
void Error(const char *text)
{
	printf("Error(%d): %s\n", BASS_ErrorGetCode(), text);
	BASS_Free();
	ExitProcess(0);
}

DWORD CALLBACK WriteStream(HSTREAM handle, short *buffer, DWORD length, void *user)
{
	int k;
	memset(buffer, 0, length);
	ModelProcess(buffer,length/ sizeof(short));
	return length;
}

//RtMidi: don't forget to define __WINDOWS_MM__ and by the libs winmmlib
void main(int argc, char **argv)
{
	INPUT_RECORD keyin;
	DWORD r, buflen;
	printf("XREModular ----------------\n");

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		printf("An incorrect version of BASS.DLL was loaded");
		return;
	}
	ModelCreate("Super52 Out-01-16"); // Your Midi In device
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10); // 10ms update period

	// initialize default output device (and measure latency)
	if (!BASS_Init(-1, 44100, BASS_DEVICE_LATENCY, 0, NULL))
		Error("Can't initialize device");

	BASS_SetConfig(BASS_CONFIG_BUFFER, 50); // set default/maximum buffer length to 200ms
	BASS_GetInfo(&info);
	if (!info.freq) info.freq = 44100; // if the device's output rate is unknown, default to 44100 Hz
	stream = BASS_StreamCreate(info.freq, 2, 0, (STREAMPROC*)WriteStream, 0); // create a stream (stereo for effects)
	buflen = 10 + info.minbuf + 1; // default buffer size = update period + 'minbuf' + 1ms extra margin
	BASS_ChannelSetAttribute(stream, BASS_ATTRIB_BUFFER, buflen / 1000.f); // apply it
	BASS_ChannelPlay(stream, FALSE); // start it

	printf("device latency: %dms\n", info.latency);
	printf("device minbuf: %dms\n", info.minbuf);
	printf("ds version: %d (effects %s)\n", info.dsver, info.dsver < 8 ? "disabled" : "enabled");
	printf("using a %dms buffer\r", buflen);
  puts("Running until you hit RETURN");
  getchar();
	BASS_Free();
}
