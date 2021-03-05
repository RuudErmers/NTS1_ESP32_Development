#pragma once

//#pragma once

typedef int  int32_t;
typedef unsigned int  uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef struct user_osc_param {
		/** Value of LFO implicitely applied to shape parameter */
		int32_t  shape_lfo;
		/** Current pitch. high byte: note number, low byte: fine (0-255) */
		uint16_t pitch;
		/** Current cutoff value (0x0000-0x1fff) */
		uint16_t cutoff;
		/** Current resonance value (0x0000-0x1fff) */
		uint16_t resonance;
		uint16_t reserved0[3];
} user_osc_param_t;

/**
* User facing osc-specific parameters
*/
typedef enum {
		/** Edit parameter 1 */
		k_user_osc_param_id1 = 0,
		/** Edit parameter 2 */
		k_user_osc_param_id2,
		/** Edit parameter 3 */
		k_user_osc_param_id3,
		/** Edit parameter 4 */
		k_user_osc_param_id4,
		/** Edit parameter 5 */
		k_user_osc_param_id5,
		/** Edit parameter 6 */
		k_user_osc_param_id6,
		/** Shape parameter */
		k_user_osc_param_shape,
		/** Alternative Shape parameter: generally available via a shift function */
		k_user_osc_param_shiftshape,
		k_num_user_osc_param_id
} user_osc_param_id_t;

#define param_val_to_f32(val) ((uint16_t)val * 9.77517106549365e-004f) 

#define k_note_mod_fscale      (0.00392156862745098f)
#define k_note_max_hz          (23679.643054f)
#define k_samplerate        (48000)
#define k_samplerate_recipf (2.08333333333333e-005f)

float osc_notehzf(uint8_t note);
float fsel(const float a, const float b, const float c);
float clampmaxfsel(const float x, const float max);
float clipmaxf(float x, float m);
float linintf(const float fr, const float x0, const float x1);
float osc_w0f_for_note(uint8_t note, uint8_t mod);
float clipminmaxf(const float min, const float x, const float max);

typedef int32_t q31_t;

#define q31_to_f32_c 4.65661287307739e-010f

#define q31_to_f32(q) ((float)(q) * q31_to_f32_c)

// BASS doesn't like -1 or +1 samples, here we do just a quick fix (after an hour of messing around)
#define f32_to_q31(f)   ((q31_t)((float)(f*0.99999) * (float)0x7FFFFFFF))


extern void OSC_INIT(uint32_t platform, uint32_t api);
extern void OSC_CYCLE(const user_osc_param_t * const params, int32_t *yn, const uint32_t frames);
extern void OSC_NOTEON(const user_osc_param_t * const params);
extern void OSC_NOTEOFF(const user_osc_param_t * const params);
extern void OSC_PARAM_NEW(uint16_t index, uint16_t value);
extern void OSC_SetParameter(uint8_t cc, uint8_t value);

