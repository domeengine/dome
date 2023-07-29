#define AUDIO_CHANNEL_START 0
#define SAMPLE_RATE 44100

global_variable const uint16_t channels = 2;
global_variable const uint16_t bytesPerSample = 4 * 2; // 4-byte float * 2 channels;

typedef struct AUDIO_ENGINE_t {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  float* scratchBuffer;
  size_t scratchBufferSize;

  TABLE pending;
  TABLE playing;
  CHANNEL_ID nextId;
} AUDIO_ENGINE;

// audio callback function
// Allows SDL to "pull" data into the output buffer
// on a separate thread. We need to be pretty efficient
// here as it holds a lock.
void AUDIO_ENGINE_mix(void*  userdata,
    Uint8* stream,
    int    outputBufferSize) {
  AUDIO_ENGINE* audioEngine = userdata;

  size_t totalSamples = outputBufferSize / bytesPerSample;

  SDL_memset(stream, 0, outputBufferSize);

  float* scratchBuffer = audioEngine->scratchBuffer;
  size_t bufferSampleSize = audioEngine->scratchBufferSize;
  TABLE_ITERATOR iter;
  TABLE_iterInit(&iter);
  CHANNEL* channel;
  while (TABLE_iterate(&(audioEngine->playing), &iter)) {
    channel = iter.value;
    if (!CHANNEL_isPlaying(channel)) {
      continue;
    }

    assert(channel != NULL);
    size_t requestServed = 0;
    float* writeCursor = (float*)(stream);
    CHANNEL_mix mixFn = channel->mix;

    while (CHANNEL_isPlaying(channel) && requestServed < totalSamples) {
      SDL_memset(scratchBuffer, 0, bufferSampleSize * bytesPerSample);
      size_t requestSize = min(bufferSampleSize, totalSamples - requestServed);
      mixFn(channel->ref, scratchBuffer, requestSize);
      requestServed += requestSize;
      float* copyCursor = scratchBuffer;
      float* endPoint = copyCursor + bufferSampleSize * channels;
      for (; copyCursor < endPoint; copyCursor++) {
        *(writeCursor++) += *copyCursor;
      }
    }
  }
}

internal AUDIO_ENGINE*
AUDIO_ENGINE_init(void) {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  AUDIO_ENGINE* engine = malloc(sizeof(AUDIO_ENGINE));

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

  engine->scratchBuffer = calloc(AUDIO_BUFFER_SIZE, bytesPerSample);
  if (engine->scratchBuffer != NULL) {
    engine->scratchBufferSize = AUDIO_BUFFER_SIZE;
  }

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);

  TABLE_init(&(engine->pending));
  TABLE_init(&(engine->playing));

  return engine;
}

internal bool
AUDIO_ENGINE_get(AUDIO_ENGINE* engine, CHANNEL_REF* ref, CHANNEL** channel) {
  CHANNEL_ID id = ref->id;
  if (id == 0) {
    return false;
  }
  bool result = TABLE_get(&engine->playing, id, channel);
  if (!result) {
    result = TABLE_get(&engine->pending, id, channel);
  }
  return result;
}

internal void
AUDIO_ENGINE_lock(AUDIO_ENGINE* engine) {
  SDL_LockAudioDevice(engine->deviceId);
}

internal void
AUDIO_ENGINE_unlock(AUDIO_ENGINE* engine) {
  SDL_UnlockAudioDevice(engine->deviceId);
}

internal CHANNEL_REF
AUDIO_ENGINE_channelInit(
    AUDIO_ENGINE* engine,
    CHANNEL_mix mix,
    CHANNEL_callback update,
    CHANNEL_callback finish,
    void* userdata) {

  CHANNEL_ID id = engine->nextId++;
  CHANNEL_REF ref = {
    .id = id,
    .engine = engine
  };
  CHANNEL channel = {
    .state = CHANNEL_INITIALIZE,
    .mix = mix,
    .update = update,
    .finish = finish,
    .userdata = userdata,
    .ref = ref
  };
  TABLE_set(&engine->pending, id, channel);

  return ref;
}

internal void
AUDIO_ENGINE_update(AUDIO_ENGINE* engine, WrenVM* vm) {
  TABLE_ITERATOR iter;
  TABLE_iterInit(&iter);
  CHANNEL* channel;
  AUDIO_ENGINE_lock(engine);
  TABLE_addAll(&engine->playing, &engine->pending);
  while (TABLE_iterate(&(engine->playing), &iter)) {
    channel = iter.value;
    if (channel->update != NULL) {
      channel->update(channel->ref, vm);
    }
    if (channel->state == CHANNEL_STOPPED) {
      if (channel->finish != NULL) {
        channel->finish(channel->ref, vm);
      }
      TABLE_delete(&engine->playing, channel->ref.id);
    }
  }
  AUDIO_ENGINE_unlock(engine);
  TABLE_free(&engine->pending);
  //  DEBUG_LOG("Capacity: %u / %u", engine->playing.items, (engine->playing).capacity);
}

internal void
AUDIO_ENGINE_stop(AUDIO_ENGINE* engine, CHANNEL_REF* ref) {
  CHANNEL* channel;
  AUDIO_ENGINE_get(engine, ref, &channel);
  if (channel != NULL) {
    CHANNEL_requestStop(channel);
  }
}

internal void*
AUDIO_ENGINE_getData(AUDIO_ENGINE* engine, CHANNEL_REF* ref) {
  CHANNEL* channel;
  AUDIO_ENGINE_get(engine, ref, &channel);
  if (channel != NULL) {
    return CHANNEL_getData(channel);
  }
  return NULL;
}

internal void
AUDIO_ENGINE_stopAll(AUDIO_ENGINE* engine) {
  TABLE_ITERATOR iter;
  TABLE_iterInit(&iter);
  CHANNEL* channel;
  while (TABLE_iterate(&(engine->playing), &iter)) {
    channel = iter.value;
    CHANNEL_requestStop(channel);
  }
  TABLE_iterInit(&iter);
  while (TABLE_iterate(&(engine->pending), &iter)) {
    channel = iter.value;
    CHANNEL_requestStop(channel);
  }
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
AUDIO_ENGINE_free(AUDIO_ENGINE* engine) {
  // We might need to free contained audio here
  AUDIO_ENGINE_halt(engine);
  free(engine->scratchBuffer);
  TABLE_free(&engine->playing);
  TABLE_free(&engine->pending);
}

