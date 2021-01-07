#define AUDIO_CHANNEL_START 0
#define SAMPLE_RATE 44100

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
  uintmax_t id;
  char* soundId;
  volatile bool enabled;
  bool stopRequested;
  void* context;
  WrenHandle* handle;
  CHANNEL_mix mix;
  CHANNEL_callback update;
  CHANNEL_callback finish;
} GENERIC_CHANNEL;

struct AUDIO_CHANNEL_PROPS {
  // Control variables
  bool loop;
  // Playback variables
  float volume;
  float pan;
  // Position is the sample value to play next
  volatile size_t position;
  bool resetPosition;
};

typedef struct {
  GENERIC_CHANNEL core;
  struct AUDIO_CHANNEL_PROPS current;
  struct AUDIO_CHANNEL_PROPS new;
  float actualVolume;
  float actualPan;
  bool fade;

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
  uintmax_t nextId;
} AUDIO_ENGINE;

const uint16_t channels = 2;
const uint16_t bytesPerSample = 4; // 4-byte float * 2 channels;

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
AUDIO_CHANNEL_commit(AUDIO_CHANNEL* channel) {
  size_t position = channel->current.position;
  if (channel->new.resetPosition) {
    position = channel->new.position;
    channel->new.resetPosition = false;
  }
  channel->current = channel->new;
  channel->current.position = position;
  channel->new = channel->current;
}

internal void
AUDIO_CHANNEL_update(WrenVM* vm, void* gChannel) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  switch (channel->core.state) {
    case CHANNEL_INITIALIZE:
      channel->core.state = CHANNEL_TO_PLAY;
      // Fallthrough
    case CHANNEL_DEVIRTUALIZE:
    case CHANNEL_TO_PLAY:
      if (channel->core.state == CHANNEL_DEVIRTUALIZE) {
        // We might do special things to de-virtualize a channel
      }
      if (channel->audio == NULL) {
        channel->core.state = CHANNEL_LOADING;
        break;
      }
      // We assume data is loaded by now.
      channel->core.state = CHANNEL_PLAYING;
      channel->core.enabled = true;
      AUDIO_CHANNEL_commit(channel);
      break;
    case CHANNEL_LOADING:
      if (channel->audio != NULL) {
        channel->core.state = CHANNEL_TO_PLAY;
      }
      break;
    case CHANNEL_PLAYING:
      AUDIO_CHANNEL_commit(channel);
      if (channel->core.stopRequested) {
        channel->core.state = CHANNEL_STOPPING;
      }
      break;
    case CHANNEL_STOPPING:
      if (channel->fade) {
        channel->new.volume -= 0.1;
      } else {
        channel->new.volume = 0;
      }
      AUDIO_CHANNEL_commit(channel);
      if (channel->new.volume <= 0) {
        channel->new.volume = 0;
        channel->core.state = CHANNEL_STOPPED;
      }
      break;
    case CHANNEL_STOPPED:
      channel->core.enabled = false;
      AUDIO_CHANNEL_commit(channel);
      break;
    default: break;
  }
}

internal void
AUDIO_CHANNEL_mix(void* gChannel, float* stream, size_t totalSamples) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  if (channel->audio == NULL) {
    return;
  }
  AUDIO_DATA* audio = channel->audio;
  float* startReadCursor = (float*)(audio->buffer);
  float* readCursor = startReadCursor + channel->current.position * channels;
  float* writeCursor = stream;
  size_t length = audio->length;

  size_t samplesToWrite = channel->current.loop ? totalSamples : min(totalSamples, length - channel->current.position);
  float volume = channel->current.volume;
  float targetPan = channel->current.pan;
  float actualVolume = channel->actualVolume;
  float actualPan = channel->actualPan;


  for (size_t i = 0; i < samplesToWrite; i++) {
    // We have to lerp the volume and pan change across the whole sample buffer
    // or we get a clicking sound.
    float f = i / (float)samplesToWrite;
    float currentVolume = lerp(actualVolume, volume, f);
    float currentPan = lerp(actualPan, targetPan, f);
    float pan = (currentPan + 1.0f) * M_PI / 4.0f; // Channel pan is [-1,1] real pan needs to be [0,1]

    // We have to advance the cursor after each read and write
    // Read/Write left
    *(writeCursor++) += *(readCursor++) * cos(pan) * currentVolume;
    // Read/Write right
    *(writeCursor++) += *(readCursor++) * sin(pan) * currentVolume;

    channel->current.position++;
    if (channel->current.position >= length) {
      if (channel->current.loop) {
        channel->current.position = 0;
        readCursor = startReadCursor;
      } else {
        break;
      }
    }
  }
  channel->actualVolume = channel->current.volume;
  channel->actualPan = channel->current.pan;
  channel->core.enabled = (channel->current.loop || channel->current.position < length);
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
}

internal float* resample(float* data, size_t srcLength, uint64_t srcFrequency, uint64_t targetFrequency, size_t* destLength);
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
  // Process incoming values into a mixable format
  for (uint32_t i = 0; i < data->length; i++) {
    data->buffer[i * channels] = (float)(tempBuffer[i * data->spec.channels]) / INT16_MAX;
    if (data->spec.channels == 1) {
      data->buffer[i * channels + 1] = (float)(tempBuffer[i * data->spec.channels]) / INT16_MAX;
    } else {
      data->buffer[i * channels + 1] = (float)(tempBuffer[i * data->spec.channels + 1]) / INT16_MAX;
    }
  }
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* audioEngine = engine->audioEngine;
  if (data->spec.freq != audioEngine->spec.freq) {
    size_t newLength;
    void* oldPtr = data->buffer;
    data->buffer = resample(data->buffer, data->length, data->spec.freq, audioEngine->spec.freq, &newLength);
    data->length = newLength;
    free(oldPtr);
  }
  // free the intermediate buffers
  if (data->audioType == AUDIO_TYPE_WAV) {
    SDL_FreeWAV((uint8_t*)tempBuffer);
  } else if (data->audioType == AUDIO_TYPE_OGG) {
    free(tempBuffer);
  }
  if (DEBUG_MODE) {
    DEBUG_printAudioSpec(engine, data->spec, data->audioType);
  }
}


internal float*
resample(float* data, size_t srcLength, uint64_t srcFrequency, uint64_t targetFrequency, size_t* destLength) {
  // Compute GCD of both frequencies
  uint64_t divisor = gcd(srcFrequency, targetFrequency);

  uint64_t L = targetFrequency / divisor;
  uint64_t M = srcFrequency / divisor;

  size_t sampleCount = srcLength;

  size_t tempSampleCount = sampleCount * L;
  size_t tempLength = tempSampleCount * channels;
  float* tempData = calloc(tempLength, sizeof(float));
  if (tempData == NULL) {
    return NULL;
  }


  size_t destSampleCount = ceil(tempSampleCount / M) + 1;
  *destLength = destSampleCount;
  float* destData = malloc(destSampleCount * bytesPerSample);
  if (destData == NULL) {
    return NULL;
  }
  // Space out samples in temp data
  float* sampleCursor = data;
  float* writeCursor = tempData;
  for (size_t i = 0; i < sampleCount * L; i++) {
    size_t index = channels * (i / L);
    *(writeCursor++) = sampleCursor[index];
    *(writeCursor++) = sampleCursor[index + 1];
  }

  // TODO: Low-pass filter over the data (optional - but recommended)

  // decimate by M
  sampleCursor = tempData;
  writeCursor = destData;

  for(size_t i = 0; i < tempSampleCount; i += M) {
    *(writeCursor++) = sampleCursor[i*2];
    *(writeCursor++) = sampleCursor[i*2+1];
  }

  free(tempData);
  return destData;
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

  // zero is reserved for uninitialized.
  engine->nextId = 1;

  // SETUP player
  // set the callback function
  (engine->spec).freq = SAMPLE_RATE;
  (engine->spec).format = AUDIO_F32LSB;
  (engine->spec).channels = channels; // TODO: consider mono/stereo
  (engine->spec).samples = AUDIO_BUFFER_SIZE;
  (engine->spec).callback = AUDIO_ENGINE_mix;
  (engine->spec).userdata = engine;

  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, 0);
  // TODO: Handle if we can't get a device!

  engine->scratchBuffer = calloc(AUDIO_BUFFER_SIZE, sizeof(float) * channels);
  if (engine->scratchBuffer != NULL) {
    engine->scratchBufferSize = AUDIO_BUFFER_SIZE;
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
  channel->id = engine->nextId++;
}

internal void
AUDIO_ENGINE_push(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  GENERIC_CHANNEL* channel = wrenGetSlotForeign(vm, 1);
  channel->handle = wrenGetSlotHandle(vm, 1);
  channel->enabled = true;
  AUDIO_ENGINE_pushChannel(data, channel);
  wrenSetSlotDouble(vm, 0, channel->id);
}

internal void
AUDIO_ENGINE_update(WrenVM* vm) {
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
    }
  }

  pending = CHANNEL_LIST_resize(pending, waitCount + next);

  AUDIO_ENGINE_lock(data);
  // transpose the two lists
  data->playing = pending;
  data->pending = playing;
  assert(data->playing->count == (waitCount + next));

  for (size_t i = 0; i < data->playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)data->playing->channels[i];
    assert(channel != NULL);
    if (channel->update != NULL) {
      channel->update(vm, channel);
    }
  }
  AUDIO_ENGINE_unlock(data);

  for (size_t i = 0; i < data->pending->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)data->pending->channels[i];
    assert(channel != NULL);
    if (channel->enabled == false) {
      channel->enabled = false;
      if (channel->finish != NULL) {
        channel->finish(vm, channel);
      }
    }
  }
  data->pending = CHANNEL_LIST_resize(data->pending, 0);
  assert(data->pending->count == 0);

  // Safety - Make sure we don't misuse these pointers.
  pending = NULL;
  playing = NULL;
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
AUDIO_ENGINE_stopAll(AUDIO_ENGINE* engine) {
  CHANNEL_LIST* playing = engine->playing;
  for (size_t i = 0; i < playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)playing->channels[i];
    channel->stopRequested = true;
  }
}

internal void
AUDIO_ENGINE_stop(AUDIO_ENGINE* engine, uintmax_t id) {
  CHANNEL_LIST* playing = engine->playing;
  for (size_t i = 0; i < playing->count; i++) {
    GENERIC_CHANNEL* channel = (GENERIC_CHANNEL*)playing->channels[i];
    if (channel->id == id) {
      channel->stopRequested = true;
    }
  }
}

internal void
AUDIO_ENGINE_wrenStopAll(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* audioEngine = engine->audioEngine;
  AUDIO_ENGINE_stopAll(audioEngine);
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
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  ASSERT_SLOT_TYPE(vm, 1, STRING, "sound id");
  const char* soundId = wrenGetSlotString(vm, 1);
  size_t len = strlen(soundId);
  channel->core.soundId = malloc((1 + len) * sizeof(char));
  strcpy(channel->core.soundId, soundId);
  channel->core.soundId[len] = '\0';

  struct AUDIO_CHANNEL_PROPS props = {0, 0, 0, 0, 0};
  channel->current = channel->new = props;
  channel->actualVolume = 0.0f;

  channel->core.state = CHANNEL_INITIALIZE;
  channel->core.enabled = true;
  channel->audio = NULL;

  channel->core.mix = AUDIO_CHANNEL_mix;
  channel->core.update = AUDIO_CHANNEL_update;
  channel->core.finish = AUDIO_CHANNEL_finish;
}

internal void
AUDIO_CHANNEL_setAudio(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  if (channel->core.state == CHANNEL_INITIALIZE) {
    ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "audio");
    channel->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 1);
    channel->audioHandle = wrenGetSlotHandle(vm, 1);
  } else {
    VM_ABORT(vm, "Cannot change audio in channel once initialized");
  }
}

internal void
AUDIO_CHANNEL_setState(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "state");
  int state = wrenGetSlotDouble(vm, 1);
  if (state <= CHANNEL_INVALID || state >= CHANNEL_LAST) {
    VM_ABORT(vm, "Setting invalid channel state");
  }
  channel->core.state = state;
}

internal void
AUDIO_CHANNEL_getState(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->core.state);
}

internal void
AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->core.id);
}

internal void
AUDIO_CHANNEL_getSoundId(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, channel->core.soundId);
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
  wrenSetSlotDouble(vm, 0, channel->new.position);
}

internal void
AUDIO_CHANNEL_setEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "enabled");
  if (channel->core.enabled) {
    channel->core.stopRequested = !wrenGetSlotBool(vm, 1);
  } else {
    channel->core.enabled = wrenGetSlotBool(vm, 1);
  }
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
  channel->new.loop = wrenGetSlotBool(vm, 1);
}

internal void
AUDIO_CHANNEL_getLoop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, channel->new.loop);
}

internal void
AUDIO_CHANNEL_setPosition(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "position");
  size_t newPosition = round(wrenGetSlotDouble(vm, 1));
  channel->new.position = mid(0, newPosition, channel->audio->length);
  channel->new.resetPosition = true;
}

internal void
AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "volume");
  channel->new.volume = fmax(0, wrenGetSlotDouble(vm, 1));
}

internal void
AUDIO_CHANNEL_getVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->new.volume);
}

internal void
AUDIO_CHANNEL_setPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "pan");
  channel->new.pan = fmid(-1.0, wrenGetSlotDouble(vm, 1), 1.0f);
}

internal void
AUDIO_CHANNEL_getPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->new.pan);
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
