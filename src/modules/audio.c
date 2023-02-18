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
  } else if (strncmp(fileBuffer, "fLaC", 4) == 0) {
    data->audioType = AUDIO_TYPE_FLAC;
    unsigned int channelsInFile = 0;
    unsigned int freq = 0;
    drflac_uint64 totalFrameCount;
    tempBuffer = drflac_open_memory_and_read_pcm_frames_s16(fileBuffer, length, &channelsInFile, &freq, &totalFrameCount, NULL);
    if (tempBuffer == NULL) {
      VM_ABORT(vm, "Invalid FLAC file");
      return;
    }

    data->spec.channels = channelsInFile;
    data->spec.freq = freq;
    data->spec.format = AUDIO_F32LSB;
    data->length = totalFrameCount;
  } else if (strncmp(fileBuffer, "ID3", 3) == 0) {
    data->audioType = AUDIO_TYPE_MP3;
    drmp3_config config = { 0 };
    drmp3_uint64 totalFrameCount;
    tempBuffer = drmp3_open_memory_and_read_pcm_frames_s16(fileBuffer, length, &config, &totalFrameCount, NULL);
    if (tempBuffer == NULL) {
      VM_ABORT(vm, "Invalid MP3 file");
      return;
    }

    data->spec.channels = config.channels;
    data->spec.freq = config.sampleRate;
    data->spec.format = AUDIO_F32LSB;
    data->length = totalFrameCount;
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
    size_t newLength = 0;
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
  } else if (data->audioType == AUDIO_TYPE_FLAC) {
    free(tempBuffer);
  } else if (data->audioType == AUDIO_TYPE_MP3) {
    free(tempBuffer);
  }
  if (DEBUG_MODE) {
    DEBUG_printAudioSpec(engine, data->spec, data->audioType);
  }
}


internal void
AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    if (audioData->audioType == AUDIO_TYPE_WAV
        || audioData->audioType == AUDIO_TYPE_OGG
        || audioData->audioType == AUDIO_TYPE_FLAC
        || audioData->audioType == AUDIO_TYPE_MP3) {
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
AUDIO_CHANNEL_stop(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

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
  while (TABLE_iterate(&(audioEngine->playing), &iter)) {
    channel = iter.value;
    channel->state = CHANNEL_STOPPED;
    if (channel->finish != NULL) {
      channel->finish(channel->ref, vm);
    }
  }
  TABLE_iterInit(&iter);
  while (TABLE_iterate(&(audioEngine->pending), &iter)) {
    channel = iter.value;
    channel->state = CHANNEL_STOPPED;
    if (channel->finish != NULL) {
      channel->finish(channel->ref, vm);
    }
  }
}


internal void
AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 2);
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(CHANNEL_REF));
  ASSERT_SLOT_TYPE(vm, 1, STRING, "sound id");

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  const char* soundId = wrenGetSlotString(vm, 1);
  *ref = AUDIO_CHANNEL_new(data, soundId);
}

internal void
AUDIO_CHANNEL_setAudio(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

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
  } else {
    VM_ABORT(vm, "Cannot change audio in channel once initialized");
  }
}

internal void
AUDIO_CHANNEL_setState(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);
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
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

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
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, ref->id);
}

internal void
AUDIO_CHANNEL_getSoundId(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

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
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

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
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, AUDIO_ENGINE_getPosition(data, ref));
}

internal void
AUDIO_CHANNEL_setLoop(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  ASSERT_SLOT_TYPE(vm, 1, BOOL, "loop");
  AUDIO_ENGINE_setLoop(data, ref, wrenGetSlotBool(vm, 1));
}

internal void
AUDIO_CHANNEL_getLoop(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, AUDIO_ENGINE_getLoop(data, ref));
}

internal void
AUDIO_CHANNEL_setPosition(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  ASSERT_SLOT_TYPE(vm, 1, NUM, "position");
  AUDIO_ENGINE_setPosition(data, ref, round(wrenGetSlotDouble(vm, 1)));
}

internal void
AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  ASSERT_SLOT_TYPE(vm, 1, NUM, "volume");
  AUDIO_ENGINE_setVolume(data, ref, wrenGetSlotDouble(vm, 1));
}

internal void
AUDIO_CHANNEL_getVolume(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  wrenSetSlotDouble(vm, 0, AUDIO_ENGINE_getVolume(data, ref));
}


internal void
AUDIO_CHANNEL_setPan(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  ASSERT_SLOT_TYPE(vm, 1, NUM, "pan");
  AUDIO_ENGINE_setPan(data, ref, wrenGetSlotDouble(vm, 1));
}

internal void
AUDIO_CHANNEL_getPan(WrenVM* vm) {
  CHANNEL_REF* ref = (CHANNEL_REF*)wrenGetSlotForeign(vm, 0);

  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, AUDIO_ENGINE_getPan(data, ref));
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

