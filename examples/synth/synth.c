#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "dome.h"
#include <math.h>

#include <stdatomic.h>

static DOME_API_v0* api;
static AUDIO_API_v0* audio;
static WREN_API_v0* wren;

static const char* source = "class Synth {\n"
                    "foreign static setTone(octave, note)\n"
                    "foreign static volume=(v)\n"
                    "foreign static volume\n"
                    "foreign static playTone(time)\n"
                    "foreign static noteOn()\n"
                    "foreign static noteOff()\n"
                  "}";




static _Atomic(double) globalTime = 0.0;
// sample step
static double step = 1.0f / 44100.0f;

static CHANNEL_REF ref;

typedef struct {
  double attack;
  double decay;
  double release;

  double startAmp;
  double sustainAmp;
  double triggerOnTime;
  double triggerOffTime;
  atomic_bool playing;
} ENVELOPE;

typedef struct {
  float phase;
  float volume;
  float octave;
  float note;
  float length;
  bool active;
  ENVELOPE env;
} SYNTH;
static SYNTH synth;

float envelope(ENVELOPE* env, double time) {
  double amp = 0.0;
  double lifeTime;

  if (env->playing) {
    lifeTime = time - env->triggerOnTime;
    // ADS
    if (lifeTime <= env->attack) {
      amp = (lifeTime / env->attack) * env->startAmp;
    } else if (env->attack < lifeTime && lifeTime < (env->decay + env->attack)) {
      amp = ((lifeTime - env->attack) / env->decay) * (env->sustainAmp - env->startAmp) + env->startAmp;
    } else {
      amp = env->sustainAmp;
    }
  } else {
    // R
    lifeTime = time - env->triggerOffTime;
    amp = (1 - (lifeTime / env->release)) * env->sustainAmp;
//    dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
  }
  if (amp < 0.0001) {
    amp = 0;
  }

  return amp;
}


float AdvanceOscilator_Saw(float* fPhase, float frequency, float fSampleRate) {
  *fPhase += frequency/fSampleRate;

	while(*fPhase > 1.0f)
		*fPhase -= 1.0f;

	while(*fPhase < 0.0f)
		*fPhase += 1.0f;

	return (*fPhase * 2.0f) - 1.0f;
}

float AdvanceOscilator_Triangle(float* fPhase, float frequency, float fSampleRate)
{
	*fPhase += frequency/fSampleRate;

	while(*fPhase > 1.0f)
		*fPhase -= 1.0f;

	while(*fPhase < 0.0f)
		*fPhase += 1.0f;

	float fRet;
	if(*fPhase <= 0.5f)
		fRet=*fPhase*2;
	else
		fRet=(1.0f - *fPhase)*2;

	return (fRet * 2.0f) - 1.0f;
}

float AdvanceOscilator_Square(float* fPhase, float fFrequency, float fSampleRate)
{
	*fPhase += fFrequency/fSampleRate;

	while(*fPhase >= 1)
		*fPhase -= 1;

	while(*fPhase < 0)
		*fPhase += 1.0;

	return *fPhase > 0.5 ? -1.0 : 1.0;
}
float AdvanceOscilator_Sine(float* fPhase, float fFrequency, float fSampleRate)
{
	*fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(*fPhase >= 2 * (float)M_PI)
		*fPhase -= 2 * (float)M_PI;

	while(*fPhase < 0)
		*fPhase += 2 * (float)M_PI;

	return sin(*fPhase);
}

#define C4 261.68

float getNoteFrequency(float octave, float noteIndex) {
  return C4 * pow(2, (((octave - 4) * 12) + noteIndex) / 12.0f);
}

PLUGIN_method(setTone, ctx, vm) {
  synth.octave = GET_NUMBER(1);
  synth.note = GET_NUMBER(2);
}

PLUGIN_method(noteOn, ctx, vm) {
  synth.active = true;
  if (!synth.env.playing) {
    synth.env.triggerOnTime = globalTime;
    synth.env.playing = true;
  }
}

PLUGIN_method(noteOff, ctx, vm) {
  synth.env.triggerOffTime = globalTime;
  synth.env.playing = false;
}
void SYNTH_mix(CHANNEL_REF ref, float* buffer, size_t requestedSamples) {
  float note = getNoteFrequency(synth.octave, synth.note);
  if (!synth.active) {
    globalTime += step * requestedSamples;
    return;
  }
  for (size_t i = 0; i < requestedSamples; i++) {
    /*
    if ((globalTime - synth.start) > synth.length) {
      break;
    }
    */
    globalTime += step;
    float s = AdvanceOscilator_Saw(&synth.phase, note, 44100) * envelope(&synth.env, globalTime);
    buffer[2*i] = s * synth.volume;
    buffer[2*i+1] = s * synth.volume;
  }
}

void SYNTH_update(CHANNEL_REF ref, WrenVM* vm) {
  //if ((globalTime - synth.start) > synth.length) {
  // }
}
void SYNTH_finish(CHANNEL_REF ref, WrenVM* vm) {

}

PLUGIN_method(setVolume, ctx, vm) {
  synth.volume = fmax(0, GET_NUMBER(1));
}
PLUGIN_method(getVolume, ctx, vm) {
  RETURN_NUMBER(synth.volume);
}

PLUGIN_method(playTone, ctx, vm) {
  synth.length = GET_NUMBER(1);

  printf("begin");
  printf("Octave: %f - Note: %f - Frequency: %f\n", synth.octave, synth.note, getNoteFrequency(synth.octave, synth.note));
}

DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getApi, DOME_Context ctx) {
  api = DOME_getAPI(API_DOME, DOME_API_VERSION);
  audio = DOME_getAPI(API_AUDIO, DOME_API_VERSION);
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  printf("init hook triggered\n");
  DOME_registerModule(ctx, "synth", source);
  DOME_registerFn(ctx, "synth", "static Synth.setTone(_,_)", setTone);
  DOME_registerFn(ctx, "synth", "static Synth.volume=(_)", setVolume);
  DOME_registerFn(ctx, "synth", "static Synth.volume", getVolume);
  DOME_registerFn(ctx, "synth", "static Synth.playTone(_)", playTone);
  DOME_registerFn(ctx, "synth", "static Synth.noteOn()", noteOn);
  DOME_registerFn(ctx, "synth", "static Synth.noteOff()", noteOff);
  ref = audio->channelCreate(ctx,
      SYNTH_mix,
      SYNTH_update,
      SYNTH_finish,
      &synth
  );
  ENVELOPE env = {
    .attack = 0.02,
    .decay = 0.01,
    .release = 0.02,
    .startAmp = 1.0,
    .sustainAmp = 0.8,
    .triggerOnTime = 0,
    .triggerOffTime = 0,
    .playing = false
  };
  synth.env = env;
  synth.volume = 0.1;
  synth.phase = 0;
  synth.octave = 4;
  synth.note = 0;
  synth.length = 0;

  audio->setState(ref, CHANNEL_PLAYING);

  return DOME_RESULT_SUCCESS;
}

DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
