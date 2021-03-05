/*
  ==============================================================================

    XMoogFilter.h
    Created: 15 Sep 2017 3:29:36pm
    Author:  Ruud

  ==============================================================================
*/

#pragma once

class XMoogFilter 
{
public:
	XMoogFilter();
	float process(float input);
	void reset();
	void setCutoff(float cutoff);
	void setResonance(float resonance);
	void setSampleRate(float value);

private:
	float FCutoff; float FResonance;
	float FSampleRate;
	float y1; float y2; float y3; float y4; float oldx; float oldy1; float oldy2; float oldy3; float x; float f; float p; float k; float t; float t2; float r;
	void calc();

};
