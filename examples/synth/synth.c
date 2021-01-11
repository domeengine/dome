#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "dome.h"
#include <math.h>

static DOME_API_v0* api;
static AUDIO_API_v0* audio;
static WREN_API_v0* wren;

static const char* source = "class Synth {\n"
                    "foreign static setTone(octave, note)\n"
                    "foreign static volume=(v)\n"
                    "foreign static volume\n"
                    "foreign static playTone(time)\n"
                  "}";




static double globalTime = 0.0;
static double step = 1.0f / 60.0f;

static CHANNEL_REF ref;
typedef struct {
  float volume;
  float phase;
  float octave;
  float note;
  double start;
  double length;
} SYNTH;
static SYNTH synth;


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

#define A4 440

float getNoteFrequency(float octave, float noteIndex) {
  return A4 * pow(2, (((octave - 4) * 12) + noteIndex) / 12.0f);
}

PLUGIN_method(setTone, ctx, vm) {
  synth.octave = GET_NUMBER(1);
  synth.note = GET_NUMBER(2);
}

void SYNTH_mix(CHANNEL_REF ref, float* buffer, size_t requestedSamples) {
  if ((globalTime - synth.start) > synth.length) {
    return;
  }
  float note = getNoteFrequency(synth.octave, synth.note);
  for (size_t i = 0; i < requestedSamples; i++) {
    float s = AdvanceOscilator_Square(&synth.phase, note, 44100);
    buffer[2*i] = s * synth.volume;
    buffer[2*i+1] = s * synth.volume;
  }
}

void SYNTH_update(CHANNEL_REF ref, WrenVM* vm) {
  if ((globalTime - synth.start) > synth.length) {
//    audio->setState(ref, CHANNEL_STOPPED);
  }
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
  synth.start = globalTime;
  synth.length = GET_NUMBER(1);

  printf("begin");
  printf("Octave: %f - Note: %f - Frequency: %f\n", synth.octave, synth.note, getNoteFrequency(synth.octave, synth.note));
}

DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getApi, DOME_Context ctx) {
  api = DOME_getAPI(API_DOME, DOME_API_VERSION);
  audio = DOME_getAPI(API_AUDIO, DOME_API_VERSION);
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  synth.volume = 0.1;
  synth.phase = 0;
  synth.octave = 4;
  synth.note = 0;
  synth.start = globalTime;
  synth.length = 0;

  printf("init hook triggered\n");
  DOME_registerModule(ctx, "synth", source);
  DOME_registerFn(ctx, "synth", "static Synth.setTone(_,_)", setTone);
  DOME_registerFn(ctx, "synth", "static Synth.volume=(_)", setVolume);
  DOME_registerFn(ctx, "synth", "static Synth.volume", getVolume);
  DOME_registerFn(ctx, "synth", "static Synth.playTone(_)", playTone);
  ref = audio->channelCreate(ctx,
      SYNTH_mix,
      SYNTH_update,
      SYNTH_finish,
      &synth
  );
  audio->setState(ref, CHANNEL_PLAYING);

  return DOME_RESULT_SUCCESS;
}

DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  globalTime += step;
  return DOME_RESULT_SUCCESS;
}
