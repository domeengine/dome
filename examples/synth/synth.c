#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "dome.h"
#include <math.h>

static DOME_API_v0* api;
static AUDIO_API_v0* audio;
static WREN_API_v0* wren;

static const char* source = "foreign class Test {\n"
                  "construct new() { System.print(\"New Class!\") }\n"
                  "foreign static bytes\n"
                  "}";



static CHANNEL_REF ref;
typedef struct {
  float phase;
} SYNTH;
static SYNTH synth;

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

float TWELVTH;
float getNoteFrequency(float noteIndex) {
  return A4 * pow(TWELVTH, noteIndex);
}

void SYNTH_mix(CHANNEL* channel, float* buffer, size_t requestedSamples) {
  float note = getNoteFrequency(1);
  for (size_t i = 0; i < requestedSamples; i++) {
    float s = AdvanceOscilator_Sine(&synth.phase, note, 44100);
    buffer[2*i] = s;
    buffer[2*i+1] = s;
  }
}

void SYNTH_update(WrenVM* vm, CHANNEL* channel) {
}
void SYNTH_finish(WrenVM* vm, CHANNEL* channel) {

}

DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getApi, DOME_Context ctx) {
  api = DOME_getAPI(API_DOME, DOME_API_VERSION);
  audio = DOME_getAPI(API_AUDIO, DOME_API_VERSION);
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);
  TWELVTH = pow(2, 1./12);

  printf("init hook triggered\n");
  ref = audio->channelCreate(ctx,
      SYNTH_mix,
      SYNTH_update,
      SYNTH_finish,
      &synth
  );
  audio->setState(ctx, ref, CHANNEL_PLAYING);

  return DOME_RESULT_SUCCESS;
}

