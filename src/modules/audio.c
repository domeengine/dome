internal void
AUDIO_CHANNEL_finish(WrenVM* vm, CHANNEL* base) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  assert(channel != NULL);
  if (channel->audioHandle != NULL) {
    // DEBUG_LOG("releasing handle %p", channel->audioHandle);
    wrenReleaseHandle(vm, channel->audioHandle);
    channel->audioHandle = NULL;
  }
  free(channel->soundId);
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
AUDIO_CHANNEL_update(WrenVM* vm, CHANNEL* base) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  switch (CHANNEL_getState(base)) {
    case CHANNEL_INITIALIZE:
      CHANNEL_setState(base, CHANNEL_TO_PLAY);
      // Fallthrough
    case CHANNEL_DEVIRTUALIZE:
    case CHANNEL_TO_PLAY:
      if (CHANNEL_getState(base) == CHANNEL_DEVIRTUALIZE) {
        // We might do special things to de-virtualize a channel
      }
      if (channel->audio == NULL) {
        CHANNEL_setState(base, CHANNEL_LOADING);
        break;
      }
      // We assume data is loaded by now.
      CHANNEL_setState(base, CHANNEL_PLAYING);
      AUDIO_CHANNEL_commit(channel);
      break;
    case CHANNEL_LOADING:
      if (channel->audio != NULL) {
        CHANNEL_setState(base, CHANNEL_TO_PLAY);
      }
      break;
    case CHANNEL_PLAYING:
      AUDIO_CHANNEL_commit(channel);
      if (CHANNEL_hasStopRequested(base)) {
        CHANNEL_setState(base, CHANNEL_STOPPING);
      }
      if (CHANNEL_getEnabled(base) == false) {
        CHANNEL_setState(base, CHANNEL_STOPPED);
      }
      break;
    case CHANNEL_STOPPING:
      if (channel->fade) {
        channel->new.volume -= 0.1;
      } else {
        channel->new.volume = 0;
      }
      if (channel->new.volume <= 0) {
        channel->new.volume = 0;
        CHANNEL_setState(base, CHANNEL_STOPPED);
      }
      AUDIO_CHANNEL_commit(channel);
      break;
    case CHANNEL_STOPPED:
      CHANNEL_setEnabled(base, false);
      AUDIO_CHANNEL_commit(channel);
      break;
    default: break;
  }
}

internal void
AUDIO_CHANNEL_mix(CHANNEL* base, float* stream, size_t totalSamples) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
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
  CHANNEL_setEnabled(base, channel->current.loop || channel->current.position < length);
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


internal void
AUDIO_ENGINE_push(WrenVM* vm) {
  /*
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  channel->enabled = true;
  AUDIO_ENGINE_pushChannel(data, channel);
  */
  wrenSetSlotDouble(vm, 0, 1);
}


internal void
AUDIO_CHANNEL_stop(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  CHANNEL_requestStop(base);
}

internal void
AUDIO_ENGINE_wrenStopAll(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* audioEngine = engine->audioEngine;
  AUDIO_ENGINE_stopAll(audioEngine);
}

internal void
AUDIO_ENGINE_releaseHandles(AUDIO_ENGINE* audioEngine, WrenVM* vm) {
  TABLE_ITERATOR iter;
  TABLE_iterInit(&iter);
  CHANNEL* channel;
  while (TABLE_iterate(&(audioEngine->channels), &iter)) {
    if (iter.done) {
      break;
    }
    channel = iter.value;
    channel->enabled = false;
    if (channel->finish != NULL) {
      channel->finish(vm, channel);
    }
  }
  TABLE_iterInit(&iter);
  while (TABLE_iterate(&(audioEngine->pending), &iter)) {
    if (iter.done) {
      break;
    }
    channel = iter.value;
    channel->enabled = false;
    if (channel->finish != NULL) {
      channel->finish(vm, channel);
    }
  }
}


internal void
AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 2);
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL_REF));
  ASSERT_SLOT_TYPE(vm, 1, STRING, "sound id");

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  const char* soundId = wrenGetSlotString(vm, 1);
  *ref = AUDIO_CHANNEL_new(data, soundId);
}

internal void
AUDIO_CHANNEL_setAudio(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = CHANNEL_getData(base);
  if (CHANNEL_getState(base) < CHANNEL_PLAYING) {
    ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "audio");
    channel->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 1);
    channel->audioHandle = wrenGetSlotHandle(vm, 1);
    // DEBUG_LOG("acquiring handle %p", channel->audioHandle);
  } else {
    VM_ABORT(vm, "Cannot change audio in channel once initialized");
  }
}

internal void
AUDIO_CHANNEL_setState(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "state");

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  int state = wrenGetSlotDouble(vm, 1);
  if (state <= CHANNEL_INVALID || state >= CHANNEL_LAST) {
    VM_ABORT(vm, "Setting invalid channel state");
  }
  CHANNEL_setState(base, state);
}

internal void
AUDIO_CHANNEL_getState(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, CHANNEL_getState(base));
}

internal void
AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, ref->id);
}

internal void
AUDIO_CHANNEL_getSoundId(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, channel->soundId);
}

internal void
AUDIO_CHANNEL_getLength(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->audio->length);
}

internal void
AUDIO_CHANNEL_getPosition(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->new.position);
}

internal void
AUDIO_CHANNEL_getEnabled(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    wrenSetSlotBool(vm, 0, false);
    return;
  }
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, CHANNEL_getEnabled(base));
}

internal void
AUDIO_CHANNEL_setLoop(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "loop");
  channel->new.loop = wrenGetSlotBool(vm, 1);
}

internal void
AUDIO_CHANNEL_getLoop(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    wrenSetSlotBool(vm, 0, false);
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, channel->new.loop);
}

internal void
AUDIO_CHANNEL_setPosition(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "position");
  size_t newPosition = round(wrenGetSlotDouble(vm, 1));
  channel->new.position = mid(0, newPosition, channel->audio->length);
  channel->new.resetPosition = true;
}

internal void
AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "volume");
  channel->new.volume = fmax(0, wrenGetSlotDouble(vm, 1));
}

internal void
AUDIO_CHANNEL_getVolume(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    wrenSetSlotDouble(vm, 0, 0);
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->new.volume);
}

internal void
AUDIO_CHANNEL_setPan(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);

  ASSERT_SLOT_TYPE(vm, 1, NUM, "pan");
  channel->new.pan = fmid(-1.0, wrenGetSlotDouble(vm, 1), 1.0f);
}

internal void
AUDIO_CHANNEL_getPan(WrenVM* vm) {
  AUDIO_CHANNEL_REF* ref = (AUDIO_CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  CHANNEL* base;
  if (!AUDIO_ENGINE_get(data, ref, &base)) {
    wrenSetSlotDouble(vm, 0, 0);
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, channel->new.pan);
}

internal void
AUDIO_CHANNEL_finalize(void* data) {}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
