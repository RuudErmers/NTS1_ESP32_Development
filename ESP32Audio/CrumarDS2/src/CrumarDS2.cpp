/*
BSD 3-Clause License

Copyright (c) 2018, KORG INC.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/


#include "userosc.h"

// ESP32: DEFINE_MAIN is needed to include main procedures here
DEFINE_MAIN

#include "XCrumarSynth.h"

XCrumarSynth CrumarSynth;

void OSC_SetParameter(uint8_t cc, uint8_t value)  // extension on Korg framework
{
		CrumarSynth.setParameter(cc,value);
}

void OSC_INIT(uint32_t platform, uint32_t api)
{
  CrumarSynth.Init();
}

void OSC_CYCLE(const user_osc_param_t * const params,		int32_t *yn,		const uint32_t frames)
{
  CrumarSynth.Process(yn, frames);
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
		CrumarSynth.NoteOn(params->pitch >> 8);
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
		CrumarSynth.NoteOff();
}

void OSC_PARAM_NEW(uint16_t index, uint16_t value)
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
				break;
// Not available from this framework, see documentation		case k_user_osc_param_shiftshape:
		}
}

void midiOut(int cc, int data1, int data2)
{ int s,t,abcd,efghi,jklmn;
    s=0;
    t=0;
    abcd=(data1 >> 3) & 0xF;
    efghi=((data1 & 7) << 2) + ((data2 >> 5) & 3);
    jklmn=data2 & 0x1F;

    data1=55;
    data2= (s << 6) + (abcd << 2);
//    WriteMidi(MIDI_CC,55,data2);

    data2=(efghi << 2) | 1;
//    WriteMidi(MIDI_CC,55,data2);

    data2=(jklmn << 2) | 2;
//    WriteMidi(MIDI_CC,55,data2);
}