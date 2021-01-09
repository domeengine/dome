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
  TABLE channels;
  CHANNEL_ID nextId;
} AUDIO_ENGINE;

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

  float* scratchBuffer = audioEngine->scratchBuffer;
  size_t bufferSampleSize = audioEngine->scratchBufferSize;
  TABLE_ITERATOR iter = {0, 0, 0, NULL };
  CHANNEL* channel;
  while (TABLE_iterate(&(audioEngine->channels), &iter)) {
    if (iter.done) {
      break;
    }
    channel = iter.value;
    if (!(channel->state == CHANNEL_PLAYING
        || channel->state == CHANNEL_STOPPING
        || channel->state == CHANNEL_VIRTUALIZING)) {
      continue;
    }

    assert(channel != NULL);
    size_t requestServed = 0;
    float* writeCursor = (float*)(stream);

    while (channel->enabled && requestServed < totalSamples) {
      SDL_memset(scratchBuffer, 0, bufferSampleSize * bytesPerSample);
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

internal inline void*
CHANNEL_getData(CHANNEL* channel) {
  return channel->userdata;
}

internal inline void
CHANNEL_setState(CHANNEL* channel, CHANNEL_STATE state) {
  channel->state = state;
}
internal inline CHANNEL_STATE
CHANNEL_getState(CHANNEL* channel) {
  return channel->state;
}
internal inline void
CHANNEL_requestStop(CHANNEL* channel) {
  channel->stopRequested = true;
}

internal inline bool
CHANNEL_hasStopRequested(CHANNEL* channel) {
  return channel->stopRequested;
}

internal inline void
CHANNEL_setEnabled(CHANNEL* channel, bool enabled) {
  channel->enabled = enabled;
}
internal inline bool
CHANNEL_getEnabled(CHANNEL* channel) {
  return channel->enabled;
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

  TABLE* table = &(engine->channels);
  TABLE_init(table);
  table = &(engine->pending);
  TABLE_init(table);

  return engine;
}

internal bool
AUDIO_ENGINE_get(AUDIO_ENGINE* engine, CHANNEL_ID id, CHANNEL** channel) {
  bool result = TABLE_get(&engine->channels, id, channel);
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

internal CHANNEL_ID
AUDIO_ENGINE_channelInit(
    AUDIO_ENGINE* engine,
    CHANNEL_mix mix,
    CHANNEL_callback update,
    CHANNEL_callback finish,
    void* userdata) {

  CHANNEL_ID id = engine->nextId++;
  CHANNEL channel = {
    .state = CHANNEL_INITIALIZE,
    .enabled = true,
    .mix = mix,
    .update = update,
    .finish = finish,
    .userdata = userdata,
    .id = id
  };
  TABLE_set(&engine->pending, id, channel);

  return id;
}

internal void
AUDIO_ENGINE_update(WrenVM* vm) {
  ENGINE* data = wrenGetUserData(vm);
  AUDIO_ENGINE* engine = data->audioEngine;
  TABLE_ITERATOR iter = {0, 0, 0, NULL};
  CHANNEL* channel;
  AUDIO_ENGINE_lock(engine);
  TABLE_addAll(&engine->channels, &engine->pending);
  while (TABLE_iterate(&(engine->channels), &iter)) {
    channel = iter.value;
    if (iter.done) {
      break;
    }
    if (channel->update != NULL) {
      channel->update(vm, channel);
    }
    if (channel->state == CHANNEL_STOPPED) {
      if (channel->finish != NULL) {
        channel->finish(vm, channel);
      }
      TABLE_delete(&engine->channels, channel->id);
    }
  }
  AUDIO_ENGINE_unlock(engine);
  TABLE_free(&engine->pending);
  //  DEBUG_LOG("Capacity: %u / %u", engine->channels.items, (engine->channels).capacity);
}

internal void
AUDIO_ENGINE_stop(AUDIO_ENGINE* engine, CHANNEL_ID id) {
  CHANNEL* channel;
  AUDIO_ENGINE_get(engine, id, &channel);
  if (channel != NULL) {
    CHANNEL_requestStop(channel);
  }
}

internal void
AUDIO_ENGINE_stopAll(AUDIO_ENGINE* engine) {
  TABLE_ITERATOR iter = {0, 0, 0, NULL };
  TABLE_iterInit(&iter);
  CHANNEL* channel;
  while (TABLE_iterate(&(engine->channels), &iter)) {
    if (iter.done) {
      break;
    }
    channel = iter.value;
    CHANNEL_requestStop(channel);
  }
  TABLE_iterInit(&iter);
  while (TABLE_iterate(&(engine->pending), &iter)) {
    if (iter.done) {
      break;
    }
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
  TABLE_free(&engine->channels);
  TABLE_free(&engine->pending);
}

internal CHANNEL_ID
AUDIO_CHANNEL_new(AUDIO_ENGINE* engine, char* soundId) {

  AUDIO_CHANNEL* data = malloc(sizeof(AUDIO_CHANNEL));
  data->soundId = strdup(soundId);
  struct AUDIO_CHANNEL_PROPS props = {0, 0, 0, 0, 0};
  data->current = data->new = props;
  data->actualVolume = 0.0f;
  data->audio = NULL;

  CHANNEL_ID id = AUDIO_ENGINE_channelInit(
    engine,
    AUDIO_CHANNEL_mix,
    AUDIO_CHANNEL_update,
    AUDIO_CHANNEL_finish,
    data
  );
  return id;
}

