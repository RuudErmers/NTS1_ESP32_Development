/*
  ==============================================================================

    XMoogFilter.cpp
    Created: 15 Sep 2017 3:29:36pm
    Author:  Ruud

  ==============================================================================
*/

#include "XMoogFilter.h"
#define _USE_MATH_DEFINES
#include <cmath>

/* TMoogFilter */

XMoogFilter::XMoogFilter()
{
	FSampleRate = 44100;
	FResonance = 0;
	reset();
}

void XMoogFilter::setCutoff(float cutoff)
{
	if (FCutoff != cutoff)
	{
		FCutoff = cutoff;
		calc();
	}
}

void XMoogFilter::setResonance(float resonance)
{
	if (resonance>0.99) resonance = 0.99;
	if (FResonance != resonance)
	{
		FResonance = resonance;
		calc();
	}
}

void XMoogFilter::setSampleRate(float value)
{
	FSampleRate = value;
	calc();
}

void XMoogFilter::calc()
{
	f = 2 * FCutoff  *1.0 / FSampleRate;
	p = f*(1.8 - 0.8*f);
	// k:=p+p-1;
	// A much better tuning seems to be:
	// Huh ?? k = 2 * sin(f*M_PI*1.0 / 2) - 1;
	k = p + p - 1;
	t = (1 - p)*1.386249;
	t2 = 12 + t*t;
	r = FResonance*(t2 + 6 * t)*1.0 / (t2 - 6 * t);
}

float XMoogFilter::process(float input)
{
	float result;
	// process input
	if (input>1) input = 1;
	if (input<-1) input = -1;
	x = input - r*y4;

	//Four cascaded onepole filters (bilinear transform)
	y1 = x*p + oldx*p - k*y1;
	y2 = y1*p + oldy1*p - k*y2;
	y3 = y2*p + oldy2*p - k*y3;
	y4 = y3*p + oldy3*p - k*y4;

	//Clipper band limited sigmoid
	y4 = y4 - (y4*y4*y4)*0.166666;

	oldx = x;
	oldy1 = y1;
	oldy2 = y2;
	oldy3 = y3;
	result = y4;
	return result;
}


void XMoogFilter::reset()
{
	y1 = 0;
	y2 = 0;
	y3 = 0;
	y4 = 0;
	oldx = 0;
	oldy1 = 0;
	oldy2 = 0;
	oldy3 = 0;
	calc();
}



