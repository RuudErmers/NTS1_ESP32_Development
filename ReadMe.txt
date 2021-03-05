KorgLogue/ESP32/PC Audio framework.

This is a small framework which I wrote to make development of custom oscillators for the 
Korg NTS-1 easier. In fact, I am using this for the STM32, which is even more powerful.

The idea is to have common code for a PC/Windows implementation, a STM32 implementation and the Korg.
So you write and test on the PC, than go into the process to serve it on your embedded boards.

This framework certainly is not perfect, I am still developing.

You can use any part of this software to your likes, except where indicated.

Please look at the files 
PCKorgLogue/Square/square.cpp
ESP32Audio/Square/square.cpp
NTS1Demos/Square/square.cpp

The file are the same (allmost?) and run on the respective platforms.
For development you only need to implement 
OSC_INIT
OSC_CYCLE
OSC_PARAMETER * 

as you would do for the Korg.

As a large example I include a CrumarDS2 implementation, which I will be using for my CrumarDS2 panel.
This is a synth wave with a full synth, 2 oscillators, 2 LFOs, 2 ADSRs etcetera.

Now, for the bonus.... 

The Korg NTS1 does not have MIDI input to the Oscillator code. That is a pity, as I would like to
set the waveshape of the oscillators, lfos, etcetera, from MIDI. The only input the oscillators have are 
k_user_osc_param_shape and k_user_osc_param_shiftshape. Fortunately theses parameters are midi enabled
(thru midi_cc 54,55). I have created a protocol over midi_cc=55 which allows to send some sys-exlusive like
data to the oscillator. At the end, there is an extra function for the framework:

void OSC_SetParameter(uint8_t cc, uint8_t value);

As you can see in the CrumarDS2 project I use this extensively to set waveforms and all other parameters.
(BTW, This is a translation of a VST of mine, which is avalable also on my github account).

To create this midi extension I had to use OSC_PARAMETER to catch the k_user_osc_param_shiftshape parameter,
and therefore in your code you should replace OSC_PARAMETER with OSC_PARAMETER_NEW.
I could fix that by changing the NTS-1 platform code, but I rather do no do that.

A few details:
- The PCKorLogue code uses BASS for output and a RtMidi port for input. It was built using Visual Studio 2017 Community Edition.
- The ESP32Audio uses a simple Command framework for Midi Input. It uses PlatformIO with VS Code.
Note that you must set the baudrate to 38400, 31250 does not work.
- The NTS1 code gets compiled with MSys64. A super simple guide to setting that up for the NTS1 can be found here (post 24)
https://www.gearslutz.com/board/electronic-music-instruments-and-electronic-music-production/1306032-korg-logue-user-oscillator-programming-thread.html 
Furthermore, I use an I2S to Audio decoder, and a ESP32 Wroom. Ask me for details.

Note that there a few differnce between the sourcefiles for the different platforms, but if you have running it one framework,
running them on the others is a piece of cake.










