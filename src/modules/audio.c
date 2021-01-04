#define AUDIO_CHANNEL_START 0

typedef enum {
  CHANNEL_INVALID,
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_DEVIRTUALIZE,
  CHANNEL_LOADING,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED,
  CHANNEL_VIRTUALIZING,
  CHANNEL_VIRTUAL,
  CHANNEL_LAST
} CHANNEL_STATE;

typedef struct {
  SDL_AudioSpec spec;
  AUDIO_TYPE audioType;
  // Length is the number of LR samples
  uint32_t length;
  // Audio is stored as a stream of interleaved normalised values from [-1, 1)
  float* buffer;
} AUDIO_DATA;


typedef void (*CHANNEL_mix)(void* channel, float* buffer, size_t requestedSamples);
typedef void (*CHANNEL_callback)(WrenVM* vm, void* channel);
typedef struct {
  CHANNEL_STATE state;
  char* soundId;
  bool enabled;
  void* context;
  WrenHandle* handle;
  CHANNEL_mix mix;
  CHANNEL_callback update;
  CHANNEL_callback finish;
} GENERIC_CHANNEL;

typedef struct {
  GENERIC_CHANNEL core;
  // Control variables
  bool loop;

  // Playback variables
  float volume;
  float pan;

  // Position is the sample value to play next
  size_t position;
  size_t newPosition;
  bool resetPosition;
  AUDIO_DATA* audio;
  WrenHandle* audioHandle;
} AUDIO_CHANNEL;

typedef struct {
  size_t count;
  GENERIC_CHANNEL* channels[];
} CHANNEL_LIST;

internal CHANNEL_LIST* CHANNEL_LIST_init(size_t initialSize);
internal CHANNEL_LIST* CHANNEL_LIST_resize(CHANNEL_LIST* list, size_t channels);

typedef struct AUDIO_ENGINE_t {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  float* scratchBuffer;
  size_t scratchBufferSize;
  CHANNEL_LIST* pending;
  CHANNEL_LIST* playing;
} AUDIO_ENGINE;

const uint16_t channels = 2;
const uint16_t bytesPerSample = sizeof(float) * 2 /* channels */;

internal void
AUDIO_CHANNEL_finish(WrenVM* vm, void* gChannel) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  assert(channel != NULL);
  if (channel->core.handle != NULL) {
    wrenReleaseHandle(vm, channel->core.handle);
    channel->core.handle = NULL;
  }

  if (channel->audioHandle != NULL) {
    wrenReleaseHandle(vm, channel->audioHandle);
    channel->audioHandle = NULL;
  }
}

internal void
AUDIO_CHANNEL_update(WrenVM* vm, void* gChannel) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  if (channel->resetPosition) {
    channel->position = channel->newPosition;
    channel->resetPosition = false;
  }
}

internal void
AUDIO_CHANNEL_mix(void* gChannel, float* stream, size_t totalSamples) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  if (channel->audio == NULL) {
    return;
  }
  AUDIO_DATA* audio = channel->audio;
  float volume = channel->volume;
  float pan = (channel->pan + 1) * M_PI / 4.0; // Channel pan is [-1,1] real pan needs to be [0,1]
  float* startReadCursor = (float*)(audio->buffer);
  float* readCursor = startReadCursor + channel->position * channels;
  float* writeCursor = stream;
  size_t length = audio->length;

  size_t samplesToWrite = channel->loop ? totalSamples : min(totalSamples, length - channel->position);

  for (size_t i = 0; i < samplesToWrite; i++) {
    // We have to advance the cursor after each read and write
    // Read/Write left
    *writeCursor++ += *readCursor++ * cos(pan) * volume;
    // Read/Write right
    *writeCursor++ += *readCursor++ * sin(pan) * volume;

    channel->position++;
    if (channel->loop && channel->position >= length) {
      channel->position = 0;
      readCursor = startReadCursor;
    }
  }
  channel->core.enabled = channel->loop || channel->position < length;
}

internal void
AUDIO_ENGINE_capture(WrenVM* vm) {
  if (audioEngineClass == NULL) {
    wrenGetVariable(vm, "audio", "AudioEngine", 0);
    audioEngineClass = wrenGetSlotHandle(vm, 0);
  }
}

// audio callback function
// Allows SDL to "pull" data into the output buffer
// on a seperate thread. We need to be pretty efficient
// here as it holds a lock.
void AUDIO_ENGINE_mix(void*  userdata,
    Uint8* stream,
    int    outputBufferSize) {
  AUDIO_ENGINE* audioEngine = userdata;

  size_t totalSamples = outputBufferSize / bytesPerSample;

  SDL_memset(stream, 0, outputBufferSize);

  CHANNEL_LIST* playing = audioEngine->playing;
  size_t channelCount = playing->count;
  size_t totalEnabled = 0;

  float* scratchBuffer = audioEngine->scratchBuffer;
  size_t bufferSampleSize = audioEngine->scratchBufferSize;

  for (size_t c = 0; c < channelCount; c++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)(playing->channels[c]);
    if (channel == NULL) {
      continue;
    }
    totalEnabled++;
    size_t requestServed = 0;
    float* writeCursor = (float*)(stream);

    while (channel->enabled && requestServed < totalSamples) {
      SDL_memset(scratchBuffer, 0, bufferSampleSize * sizeof(float) * channels);
      size_t requestSize = min(bufferSampleSize, totalSamples - requestServed);
      channel->mix(channel, scratchBuffer, requestSize);
      requestServed += requestSize;
      float* copyCursor = scratchBuffer;
      float* endPoint = copyCursor + bufferSampleSize * channels;
      for (; copyCursor < endPoint; copyCursor++) {
        *(writeCursor++) += *copyCursor;
      }
    }
  }

  // Mix using tanh
  float* outputCursor = (float*)(stream);
  float* endPoint = outputCursor + totalSamples * channels;
  for (; outputCursor < endPoint; outputCursor++) {
    *outputCursor += tanh(*outputCursor);
  }
}

internal void
AUDIO_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  AUDIO_DATA* data = (AUDIO_DATA*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_DATA));
  int length;
  ASSERT_SLOT_TYPE(vm, 1, STRING, "buffer");
  const char* fileBuffer = wrenGetSlotBytes(vm, 1, &length);

  int16_t* tempBuffer;
  if (strncmp(fileBuffer, "RIFF", 4) == 0 &&
      strncmp(&fileBuffer[8], "WAVE", 4) == 0) {
    data->audioType = AUDIO_TYPE_WAV;

    // Loading the WAV file
    SDL_RWops* src = SDL_RWFromConstMem(fileBuffer, length);
    void* result = SDL_LoadWAV_RW(src, 1, &data->spec, ((uint8_t**)&tempBuffer), &data->length);
    if (result == NULL) {
      VM_ABORT(vm, "Invalid WAVE file");
      return;
    }
    data->length /= sizeof(int16_t) * data->spec.channels;
  } else if (strncmp(fileBuffer, "OggS", 4) == 0) {
    data->audioType = AUDIO_TYPE_OGG;

    int channelsInFile = 0;
    int freq = 0;
    memset(&data->spec, 0, sizeof(SDL_AudioSpec));
    // Loading the OGG file
    int32_t result = stb_vorbis_decode_memory((const unsigned char*)fileBuffer, length, &channelsInFile, &freq, &tempBuffer);
    if (result == -1) {
      VM_ABORT(vm, "Invalid OGG file");
      return;
    }
    data->length = result;

    data->spec.channels = channelsInFile;
    data->spec.freq = freq;
    data->spec.format = AUDIO_F32LSB; // AUDIO_S16LSB;
  } else {
    VM_ABORT(vm, "Audio file was of an incompatible format");
    return;
  }

  data->buffer = calloc(channels * data->length, sizeof(float));
  assert(data->buffer != NULL);
  assert(data->length != UINT32_MAX);
  // Process incoming values into an intermediate mixable format
  for (uint32_t i = 0; i < data->length; i++) {
    data->buffer[i * channels] = (float)(tempBuffer[i * data->spec.channels]) / INT16_MAX;
    if (data->spec.channels == 1) {
      data->buffer[i * channels + 1] = (float)(tempBuffer[i * data->spec.channels]) / INT16_MAX;
    } else {
      data->buffer[i * channels + 1] = (float)(tempBuffer[i * data->spec.channels + 1]) / INT16_MAX;
    }
  }
  // free the intermediate buffers
  if (data->audioType == AUDIO_TYPE_WAV) {
    SDL_FreeWAV((uint8_t*)tempBuffer);
  } else if (data->audioType == AUDIO_TYPE_OGG) {
    free(tempBuffer);
  }
  if (DEBUG_MODE) {
    ENGINE* engine = wrenGetUserData(vm);
    DEBUG_printAudioSpec(engine, data->spec, data->audioType);
  }
}

internal void
AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    if (audioData->audioType == AUDIO_TYPE_WAV || audioData->audioType == AUDIO_TYPE_OGG) {
      free(audioData->buffer);
    }
    audioData->buffer = NULL;
  }
}
internal void
AUDIO_unload(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "audio data");
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  AUDIO_finalize(data);
}

internal void
AUDIO_getLength(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, data->length);
}

internal AUDIO_ENGINE*
AUDIO_ENGINE_init(void) {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  AUDIO_ENGINE* engine = malloc(sizeof(AUDIO_ENGINE));
  engine->playing = CHANNEL_LIST_init(AUDIO_CHANNEL_START);
  engine->pending = CHANNEL_LIST_init(AUDIO_CHANNEL_START);

  // SETUP player
  // set the callback function
  (engine->spec).freq = 44100;
  (engine->spec).format = AUDIO_F32LSB;
  (engine->spec).channels = channels; // TODO: consider mono/stereo
  (engine->spec).samples = AUDIO_BUFFER_SIZE; // Consider making this configurable
  (engine->spec).callback = AUDIO_ENGINE_mix;
  (engine->spec).userdata = engine;

  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, 0);
  // TODO: Handle if we can't get a device!

  engine->scratchBuffer = calloc(AUDIO_BUFFER_SIZE, sizeof(float) * channels);
  if (engine->scratchBuffer != NULL) {
    engine->scratchBufferSize = AUDIO_BUFFER_SIZE;
    printf("Scratch buffer size: %zu\n", engine->scratchBufferSize);
  }

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);
  return engine;
}


internal CHANNEL_LIST*
CHANNEL_LIST_init(size_t initialSize) {
  CHANNEL_LIST* list = malloc(sizeof(CHANNEL_LIST));
  list->count = 0;
  list = CHANNEL_LIST_resize(list, initialSize);
  return list;
}

internal CHANNEL_LIST*
CHANNEL_LIST_resize(CHANNEL_LIST* list, size_t channels) {
  size_t current = list->count;
  list = realloc(list, sizeof(CHANNEL_LIST) + sizeof(GENERIC_CHANNEL*) * channels);
  list->count = channels;
  for (int i = current; i < channels; i++) {
    list->channels[i] = NULL;
  }
  return list;
}

internal void
AUDIO_ENGINE_lock(AUDIO_ENGINE* engine) {
  SDL_LockAudioDevice(engine->deviceId);
}

internal void
AUDIO_ENGINE_unlock(AUDIO_ENGINE* engine) {
  SDL_UnlockAudioDevice(engine->deviceId);
}


internal void
AUDIO_ENGINE_pushChannel(AUDIO_ENGINE* engine, GENERIC_CHANNEL* channel) {
  CHANNEL_LIST* list = engine->pending;
  size_t next = list->count;
  list = CHANNEL_LIST_resize(list, next + 1);
  engine->pending = list;
  list->channels[next] = channel;
}
internal void
AUDIO_ENGINE_push(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  // assert
  GENERIC_CHANNEL* channel = wrenGetSlotForeign(vm, 1);
  channel->handle = wrenGetSlotHandle(vm, 1);
  channel->enabled = true;
  AUDIO_ENGINE_pushChannel(data, channel);
}

internal void
AUDIO_ENGINE_update(WrenVM* vm) {
  // We need additional slots to parse a list
  // wrenEnsureSlots(vm, 3);
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  CHANNEL_LIST* pending = data->pending;
  CHANNEL_LIST* playing = data->playing;


  // Copy enabled channels into pending
  size_t waitCount = pending->count;
  size_t next = 0;
  pending = CHANNEL_LIST_resize(pending, waitCount + playing->count);
  for (size_t i = 0; i < playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)playing->channels[i];
    if (channel == NULL) {
      continue;
    }
    if (channel->enabled == true) {
      pending->channels[waitCount + next] = channel;
      next++;
    } else {
      if (channel->finish != NULL) {
        channel->finish(vm, channel);
      }
    }
  }


  AUDIO_ENGINE_lock(data);
  pending = CHANNEL_LIST_resize(pending, waitCount + next);
  playing = CHANNEL_LIST_resize(playing, 0);
  // transpose the two lists
  data->playing = pending;
  data->pending = playing;
  assert(data->playing->count == (waitCount + next));
  assert(data->pending->count == 0);

  for (size_t i = 0; i < data->playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)data->playing->channels[i];
    assert(channel != NULL);
    if (channel->update != NULL) {
      channel->update(vm, channel);
    }
  }
  AUDIO_ENGINE_unlock(data);
  // Safety - Make sure we don't misuse these pointers.
  pending = NULL;
  playing = NULL;

  /*
  AUDIO_ENGINE_lock(data);
  ASSERT_SLOT_TYPE(vm, 1, LIST, "channels");
  uint8_t soundCount = wrenGetListCount(vm, 1);
  data->playing = CHANNEL_LIST_resize(data->playing, soundCount);
  for (size_t i = 0; i < data->playing->count; i++) {
    if (i < soundCount) {
      wrenGetListElement(vm, 1, i, 2);
      if (wrenGetSlotType(vm, 2) != WREN_TYPE_NULL) {
        data->playing->channels[i] = wrenGetSlotForeign(vm, 2);
        AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)data->playing->channels[i];
        if (channel->resetPosition) {
          channel->position = channel->newPosition;
          channel->resetPosition = false;
        }
      }
    } else {
      data->playing->channels[i] = NULL;
    }
  }
  AUDIO_ENGINE_unlock(data);
  */
}

internal void
AUDIO_ENGINE_pause(AUDIO_ENGINE* engine) {
  SDL_PauseAudioDevice(engine->deviceId, 1);
}

internal void
AUDIO_ENGINE_resume(AUDIO_ENGINE* engine) {
  SDL_PauseAudioDevice(engine->deviceId, 0);
}

internal void
AUDIO_ENGINE_halt(AUDIO_ENGINE* engine) {
  if (engine != NULL) {
    SDL_PauseAudioDevice(engine->deviceId, 1);
    SDL_CloseAudioDevice(engine->deviceId);
  }
}

internal void
AUDIO_ENGINE_releaseHandles(AUDIO_ENGINE* engine, WrenVM* vm) {
  CHANNEL_LIST* pending = engine->pending;
  CHANNEL_LIST* playing = engine->playing;
  for (size_t i = 0; i < playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)playing->channels[i];
    channel->enabled = false;
    if (channel->finish != NULL) {
      channel->finish(vm, channel);
    }
  }
  for (size_t i = 0; i < pending->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)pending->channels[i];
    channel->enabled = false;
    if (channel->finish != NULL) {
      channel->finish(vm, channel);
    }
  }
}

internal void
AUDIO_ENGINE_free(AUDIO_ENGINE* engine) {
  // We might need to free contained audio here
  AUDIO_ENGINE_halt(engine);
  free(engine->scratchBuffer);
  free(engine->playing);
}

internal void
AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 2);
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  ASSERT_SLOT_TYPE(vm, 1, STRING, "sound id");
  const char* soundId = wrenGetSlotString(vm, 1);
  size_t len = strlen(soundId);
  data->core.soundId = malloc((1 + len) * sizeof(char));
  strcpy(data->core.soundId, soundId);
  data->core.soundId[len] = '\0';

  data->core.state = CHANNEL_INITIALIZE;
  data->core.enabled = true;
  data->loop = false;
  data->audio = NULL;

  data->core.mix = AUDIO_CHANNEL_mix;
  data->core.update = AUDIO_CHANNEL_update;
  data->core.finish = AUDIO_CHANNEL_finish;
}

internal void
AUDIO_CHANNEL_setAudio(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  if (data->core.state == CHANNEL_INITIALIZE) {
    ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "audio");
    data->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 1);
    data->audioHandle = wrenGetSlotHandle(vm, 0);
  } else {
    VM_ABORT(vm, "Cannot change audio in channel once initialized");
  }
}

internal void
AUDIO_CHANNEL_setState(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "state");
  int state = wrenGetSlotDouble(vm, 1);
  if (state <= CHANNEL_INVALID || state >= CHANNEL_LAST) {
    VM_ABORT(vm, "Setting invalid channel state");
  }
  data->core.state = state;
}

internal void
AUDIO_CHANNEL_getSoundId(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, data->core.soundId);
}

internal void
AUDIO_CHANNEL_getState(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, data->core.state);
}

internal void
AUDIO_CHANNEL_getLength(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->audio->length);
}

internal void
AUDIO_CHANNEL_getPosition(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->position);
}

internal void
AUDIO_CHANNEL_setEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "enabled");
  channel->core.enabled = wrenGetSlotBool(vm, 1);
}

internal void
AUDIO_CHANNEL_getEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, channel->core.enabled);
}

internal void
AUDIO_CHANNEL_setLoop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "loop");
  channel->loop = wrenGetSlotBool(vm, 1);
}

internal void
AUDIO_CHANNEL_getLoop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, channel->loop);
}

internal void
AUDIO_CHANNEL_setPosition(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "position");
  size_t newPosition = round(wrenGetSlotDouble(vm, 1));
  channel->newPosition = mid(0, newPosition, channel->audio->length);
  channel->resetPosition = true;
}
internal void
AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "volume");
  channel->volume = fmax(0, wrenGetSlotDouble(vm, 1));
}

internal void
AUDIO_CHANNEL_getVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->volume);
}

internal void
AUDIO_CHANNEL_setPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "pan");
  channel->pan = fmid(-1.0, wrenGetSlotDouble(vm, 1), 1.0f);
}

internal void
AUDIO_CHANNEL_getPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->pan);
}

internal void
AUDIO_CHANNEL_finalize(void* data) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)data;
  free(channel->core.soundId);
}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
