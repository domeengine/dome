typedef struct {
  SDL_AudioSpec spec;
  AUDIO_TYPE audioType;
  // Length is the number of LR samples
  uint32_t length;
  // Audio is stored as a stream of interleaved normalised values from [-1, 1)
  float* buffer;
} AUDIO_DATA;

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
  CHANNEL base;
  struct AUDIO_CHANNEL_PROPS current;
  struct AUDIO_CHANNEL_PROPS new;
  char* soundId;
  float actualVolume;
  float actualPan;
  bool fade;

  AUDIO_DATA* audio;
  WrenHandle* audioHandle;
} AUDIO_CHANNEL;


internal void
AUDIO_CHANNEL_finish(WrenVM* vm, void* gChannel) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)gChannel;
  assert(channel != NULL);
  if (channel->base.handle != NULL) {
    wrenReleaseHandle(vm, channel->base.handle);
    channel->base.handle = NULL;
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
  switch (channel->base.state) {
    case CHANNEL_INITIALIZE:
      channel->base.state = CHANNEL_TO_PLAY;
      // Fallthrough
    case CHANNEL_DEVIRTUALIZE:
    case CHANNEL_TO_PLAY:
      if (channel->base.state == CHANNEL_DEVIRTUALIZE) {
        // We might do special things to de-virtualize a channel
      }
      if (channel->audio == NULL) {
        channel->base.state = CHANNEL_LOADING;
        break;
      }
      // We assume data is loaded by now.
      channel->base.state = CHANNEL_PLAYING;
      AUDIO_CHANNEL_commit(channel);
      break;
    case CHANNEL_LOADING:
      if (channel->audio != NULL) {
        channel->base.state = CHANNEL_TO_PLAY;
      }
      break;
    case CHANNEL_PLAYING:
      AUDIO_CHANNEL_commit(channel);
      if (channel->base.stopRequested) {
        channel->base.state = CHANNEL_STOPPING;
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
        channel->base.state = CHANNEL_STOPPED;
      }
      break;
    case CHANNEL_STOPPED:
      channel->base.enabled = false;
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
  channel->base.enabled = (channel->current.loop || channel->current.position < length);
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
    data->spec.format = AUDIO_F32LSB;
  } else {
    VM_ABORT(vm, "Audio file was of an incompatible format");
    return;
  }

  data->buffer = calloc(data->length, bytesPerSample);
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
  float* tempData = calloc(tempSampleCount, bytesPerSample);
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
  list = realloc(list, sizeof(CHANNEL_LIST) + sizeof(CHANNEL*) * channels);
  list->count = channels;
  for (int i = current; i < channels; i++) {
    list->channels[i] = NULL;
  }
  return list;
}


internal void
AUDIO_ENGINE_push(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* channel = wrenGetSlotForeign(vm, 1);
  channel->handle = wrenGetSlotHandle(vm, 1);
  channel->enabled = true;
  AUDIO_ENGINE_pushChannel(data, channel);
  wrenSetSlotDouble(vm, 0, channel->id);
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
    CHANNEL* channel = (CHANNEL*)playing->channels[i];
    channel->stopRequested = true;
  }
}


internal void
AUDIO_CHANNEL_stop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->base.stopRequested = true;
}

internal void
AUDIO_ENGINE_wrenStopAll(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* audioEngine = engine->audioEngine;
  AUDIO_ENGINE_stopAll(audioEngine);
}

internal void
AUDIO_ENGINE_releaseHandles(AUDIO_ENGINE* engine, WrenVM* vm) {
  CHANNEL_LIST* pending = engine->pending;
  CHANNEL_LIST* playing = engine->playing;
  for (size_t i = 0; i < playing->count; i++) {
    CHANNEL* channel = (CHANNEL*)playing->channels[i];
    channel->enabled = false;
    if (channel->methods.finish != NULL) {
      channel->methods.finish(vm, channel);
    }
  }
  for (size_t i = 0; i < pending->count; i++) {
    CHANNEL* channel = (CHANNEL*)pending->channels[i];
    channel->enabled = false;
    if (channel->methods.finish != NULL) {
      channel->methods.finish(vm, channel);
    }
  }
}


internal void
AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 2);
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  ASSERT_SLOT_TYPE(vm, 1, STRING, "sound id");

  const char* soundId = wrenGetSlotString(vm, 1);
  channel->soundId = strdup(soundId);

  struct AUDIO_CHANNEL_PROPS props = {0, 0, 0, 0, 0};
  channel->current = channel->new = props;
  channel->actualVolume = 0.0f;

  channel->base.state = CHANNEL_INITIALIZE;
  channel->base.enabled = true;
  channel->audio = NULL;

  channel->base.methods.mix = AUDIO_CHANNEL_mix;
  channel->base.methods.update = AUDIO_CHANNEL_update;
  channel->base.methods.finish = AUDIO_CHANNEL_finish;
}

internal void
AUDIO_CHANNEL_setAudio(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  if (channel->base.state == CHANNEL_INITIALIZE) {
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
  channel->base.state = state;
}

internal void
AUDIO_CHANNEL_getState(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->base.state);
}

internal void
AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->base.id);
}

internal void
AUDIO_CHANNEL_getSoundId(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, channel->soundId);
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
AUDIO_CHANNEL_getEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, channel->base.enabled);
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
  free(channel->soundId);
}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
