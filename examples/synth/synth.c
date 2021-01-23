#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "dome.h"
#include <math.h>

#include <stdatomic.h>

static DOME_API_v0* api;
static AUDIO_API_v0* audio;
static WREN_API_v0* wren;

static const char* source = "class Synth {\n"
                    "foreign static volume=(v)\n"
                    "foreign static volume\n"
                    "foreign static playTone(frequency, time)\n"
                    "foreign static playNote(octave, note, time)\n"
                    "foreign static noteOn(octave, note)\n"
                    "foreign static noteOff()\n"
                    "foreign static storePattern(pattern)\n"
                    "foreign static playPattern()\n"
                  "}";

static volatile _Atomic(double) globalTime = 0.0;
// sample step
static double step = 1.0f / 44100.0f;

static CHANNEL_REF ref;

typedef enum {
  OSC_SINE,
  OSC_SQUARE,
  OSC_SAW,
  OSC_TRIANGLE
} OSC_TYPE;

typedef struct {
  double attack;
  double decay;
  double release;

  double startAmp;
  double sustainAmp;
  volatile double triggerOnTime;
  volatile double triggerOffTime;
  volatile atomic_bool playing;
} ENVELOPE;

typedef struct {
  double duration; // 2/4/8/16/32/64
  int8_t pitch; // 0-11, C -> B
  int8_t octave; //1-3  - Minimal support  - 0 means silence
} NOTE;

typedef struct {
  size_t count;
  NOTE notes[];
} PATTERN;

typedef enum {
  MODE_TONE,
  MODE_PATTERN,
  MODE_NOTE
} SYNTH_MODE;

typedef struct {
  OSC_TYPE type;
  float volume;
  NOTE note;
  float frequency;
  float length;
  volatile bool active;
  volatile bool loop;
  ENVELOPE env;
  size_t position;
  double startTime;

  volatile bool swapPattern;
  volatile PATTERN* pattern;
  volatile PATTERN* pendingPattern;

  volatile SYNTH_MODE mode;
} SYNTH;
static SYNTH synth;


inline bool isNoteLetter(char c) {
  return (c >= 'a' && c <= 'g') || (c >= 'A' && c <= 'G');
}
// GNU11 standards are weird about inlines
bool isNoteLetter(char c);

float w(double frequency) {
  return (2 * M_PI * frequency);
}


float phase(float frequency, float time) {
  return sin(w(frequency) * time);
}

#define C4 261.68

float getNoteFrequency(float octave, float noteIndex) {
  return C4 * pow(2, (((octave - 4) * 12) + noteIndex) / 12.0f);
}

// frequenct to Angular frequency
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
  }
  if (amp < 0.0001) {
    amp = 0;
  }

  return amp;
}

void SYNTH_init() {
  ENVELOPE env = {
    .attack = 0.02,
    .decay = 0.01,
    .release = 0.02,
    .startAmp = 1.0,
    .sustainAmp = 1.0,
    .triggerOnTime = 0,
    .triggerOffTime = 0,
    .playing = false
  };
  synth.env = env;
  synth.volume = 0.5;
  synth.type = OSC_SAW;
  synth.frequency = getNoteFrequency(4, 0);
  synth.length = 0;
  synth.loop = false;
  synth.pattern = NULL;
  synth.startTime = 0;
  synth.pendingPattern = NULL;
}

void SYNTH_advancePattern(SYNTH* synth) {
  if (synth->pattern != NULL) {
    NOTE note = synth->pattern->notes[synth->position];
    // Move through the pattern
    if ((globalTime - synth->startTime) >= note.duration) {
      synth->position++;
      if (synth->position >= synth->pattern->count) {
        if (synth->loop) {
          synth->position = 0;
        } else if (synth->pattern != NULL) {
          if (synth->pattern != synth->pendingPattern) {
            free((void*)synth->pattern);
          }
          synth->pattern = NULL;
          synth->active = false;
          synth->env.playing = false;
        }
      }
      synth->startTime = globalTime;
    }
  }
}

void SYNTH_mix(CHANNEL_REF ref, float* buffer, size_t requestedSamples) {
  float freq;
  if (synth.pattern == NULL) {
    freq = synth.frequency;
  }

  if (synth.frequency < 20 || !synth.active) {
    // We should still advance the clock when we aren't playing anything?
    globalTime += step * requestedSamples;
    SYNTH_advancePattern(&synth);
  } else {
    for (size_t i = 0; i < requestedSamples; i++) {
      NOTE note;
      if (synth.length > 0 && (globalTime - synth.startTime) >= synth.length) {
        synth.length = 0;
        synth.frequency = 0;
      }
      if (synth.pattern != NULL) {
        note = synth.pattern->notes[synth.position];
        freq = getNoteFrequency(note.octave, note.pitch);
      }

      if (freq > 20) {

        float s;
        switch (synth.type) {
          case OSC_SINE:
            s = phase(freq, globalTime);
            break;
          case OSC_SQUARE:
            s = phase(freq, globalTime) > 0 ? 1 : -1;
            break;
          case OSC_TRIANGLE:
            s = asin(phase(freq, globalTime)) * (2.0f / M_PI);
            break;
          case OSC_SAW:
            s = (2.0f / M_PI) * freq * M_PI * fmod(globalTime, 1.0 / freq) - (M_PI / 2.0f);
            break;

          default: s = 0;
        }

        s = s * envelope(&synth.env, globalTime) * synth.volume;
        buffer[2*i] = s;
        buffer[2*i+1] = s;
      }

      globalTime += step;
      SYNTH_advancePattern(&synth);
    }
  }
}


void SYNTH_activate() {
  synth.active = true;
  if (!synth.env.playing) {
    synth.env.triggerOnTime = globalTime;
    synth.env.playing = true;
  }
}

void SYNTH_update(CHANNEL_REF ref, WrenVM* vm) {
  if (synth.swapPattern) {
    if (synth.pattern != synth.pendingPattern && synth.pattern != NULL) {
      free((void*)synth.pattern);
    }
    synth.pattern = synth.pendingPattern;
    synth.position = 0;
    synth.swapPattern = false;
    synth.startTime = globalTime;
  }
}
void SYNTH_finish(CHANNEL_REF ref, WrenVM* vm) {
  if (synth.pattern != NULL) {
    free((void*)synth.pattern);
  }
  if (synth.pendingPattern != NULL && synth.pendingPattern != synth.pattern) {
    free((void*)synth.pendingPattern);
  }
  synth.pattern = NULL;
  synth.pendingPattern = NULL;
}

PLUGIN_method(playPattern, ctx, vm) {
  synth.swapPattern = true;
  synth.active = true;
  synth.env.playing = true;
  synth.env.triggerOnTime = globalTime;
  synth.startTime = globalTime;
}
PLUGIN_method(storePattern, ctx, vm) {
  const char* patternStr = strdup(GET_STRING(1));

  PATTERN* pattern = malloc(sizeof(PATTERN));
  pattern->count = 0;
  char* saveptr = NULL;
  char* token = strtok_r((char*)patternStr, " ", &saveptr);
  char buf[8];
  double bpm = 125;
  int8_t defaultDuration = 4;
  int8_t defaultOctave = 4;
  int8_t baseOctave = defaultOctave;
  while (token != NULL) {
    DOME_log(ctx, "token: %s\n", token);
    NOTE note;
    size_t len = strlen(token);

    size_t i = 0;
    int8_t d = defaultDuration;
    if (isdigit(token[i])) {
      while (i < len && isdigit(token[i])) {
        buf[i] = token[i];
        i++;
      }
      buf[i] = '\0';
      DOME_log(ctx, "%s\n", buf);
      d = atoi(buf);
    }
    note.duration = 240.0 / bpm / d;
    bool dot = token[i] == '.';
    if (dot) {
      note.duration *= 1.5;
      i++;
    }

    bool sharp = token[i] == '#';
    if (sharp) {
      i++;
    }
    char letter = token[i];
    if (isNoteLetter(letter)) {
      int k = tolower(letter) & 7;
      note.pitch = (((int)(k * 1.6) + 8 + sharp) % 12);
      i++;
    }
    note.octave = defaultOctave;
    if (i < len) {
      if (token[i] >= '1' && token[i] <= '8') {
        note.octave = baseOctave + (token[i] - '1');
      }
      if (token[i] == '_' || token[i] == '-' || token[i] == 'p' || token[i] == 'P') {
        note.octave = 0;
      }
    }
    pattern->count++;
    pattern = realloc(pattern, sizeof(PATTERN) + sizeof(NOTE) * pattern->count);
    pattern->notes[pattern->count - 1] = note;
    token = strtok_r(NULL, " ", &saveptr);
  }
  // Should be done in a safe spot
  if (synth.pendingPattern != NULL) {
    free((void*)synth.pendingPattern);
  }
  synth.pendingPattern = pattern;

  for (size_t i = 0; i < pattern->count; i++) {
    NOTE note = pattern->notes[i];
    DOME_log(ctx, "Duration: %f - Pitch: %i - Octave: %i\n", note.duration, note.pitch, note.octave);
  }
  free((void*)patternStr);
}

PLUGIN_method(noteOn, ctx, vm) {
  int octave = GET_NUMBER(1);
  int pitch = GET_NUMBER(2);
  synth.frequency = getNoteFrequency(octave, pitch);

  SYNTH_activate();
  DOME_log(ctx, "Octave: %i - Note: %i - Frequency: %f\n", octave, pitch, synth.frequency);
}

PLUGIN_method(noteOff, ctx, vm) {
  synth.env.triggerOffTime = globalTime;
  synth.env.playing = false;
}

PLUGIN_method(setVolume, ctx, vm) {
  synth.volume = fmax(0, GET_NUMBER(1));
}
PLUGIN_method(getVolume, ctx, vm) {
  RETURN_NUMBER(synth.volume);
}

PLUGIN_method(playTone, ctx, vm) {
  synth.frequency = GET_NUMBER(1);
  synth.length = GET_NUMBER(2) / 1000.0;
  synth.startTime = globalTime;

  SYNTH_activate();

  DOME_log(ctx, "Begin");
  DOME_log(ctx, "Frequency: %f\n", synth.frequency);
}

PLUGIN_method(playNote, ctx, vm) {
  int octave = GET_NUMBER(1);
  int pitch = GET_NUMBER(2);
  synth.frequency = getNoteFrequency(octave, pitch);
  synth.length = GET_NUMBER(3);
  synth.startTime = globalTime;

  SYNTH_activate();

  DOME_log(ctx, "Begin");
  DOME_log(ctx, "Octave: %i - Note: %i - Frequency: %f\n", octave, pitch, synth.frequency);
}

DOME_EXPORTED DOME_Result
PLUGIN_onInit(DOME_getAPIFunction DOME_getApi, DOME_Context ctx) {
  api = DOME_getApi(API_DOME, DOME_API_VERSION);
  audio = DOME_getApi(API_AUDIO, DOME_API_VERSION);
  wren = DOME_getApi(API_WREN, WREN_API_VERSION);

  DOME_log(ctx, "init hook triggered\n");
  const DOME_Result result = DOME_registerModule(ctx, "synth", source);
  if (result == DOME_RESULT_FAILURE) {
    return result;
  }
  DOME_registerFn(ctx, "synth", "static Synth.volume=(_)", setVolume);
  DOME_registerFn(ctx, "synth", "static Synth.volume", getVolume);
  DOME_registerFn(ctx, "synth", "static Synth.playTone(_,_)", playTone);
  DOME_registerFn(ctx, "synth", "static Synth.playNote(_,_,_)", playNote);
  DOME_registerFn(ctx, "synth", "static Synth.noteOn(_,_)", noteOn);
  DOME_registerFn(ctx, "synth", "static Synth.noteOff()", noteOff);
  DOME_registerFn(ctx, "synth", "static Synth.storePattern(_)", storePattern);
  DOME_registerFn(ctx, "synth", "static Synth.playPattern()", playPattern);
  ref = audio->channelCreate(ctx,
      SYNTH_mix,
      SYNTH_update,
      SYNTH_finish,
      &synth
  );
  SYNTH_init();

  audio->setState(ref, CHANNEL_PLAYING);

  return DOME_RESULT_SUCCESS;
}


DOME_EXPORTED DOME_Result
PLUGIN_preUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
