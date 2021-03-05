#ifndef __SIMPLE_DELAY_H
#define __SIMPLE_DELAY_H


void Delay_Init(float delay_samples, float dfb, float dfw, float dmix);
void Delay_set_fb(float val);
void Delay_set_fw(float val);
void Delay_set_mix(float val);
void Delay_set_delay(float n_delay);
float Delay_get_fb(void);
float Delay_get_fw(void);
float Delay_get_mix(void);
float Delay_task(float xin);

/*
This interface defines the delay object
*/
  struct fract_delay {
		float d_mix;       /*delay blend parameter*/
		int d_samples;	/*delay duration in samples*/
		float d_fb;	    /*feedback volume*/
		float d_fw;	    /*delay tap mix volume*/
		float n_fract;     /*fractional part of the delay*/
		float *rdPtr;      /*delay read pointer*/
		float *wrtPtr;     /*delay write pointer*/
};
#endif