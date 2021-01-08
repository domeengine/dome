#define AUDIO_CHANNEL_START 0
#define SAMPLE_RATE 44100


internal CHANNEL_LIST* CHANNEL_LIST_init(size_t initialSize);
internal CHANNEL_LIST* CHANNEL_LIST_resize(CHANNEL_LIST* list, size_t channels);

global_variable const uint16_t channels = 2;
global_variable const uint16_t bytesPerSample = 4 * 2; // 4-byte float * 2 channels;

typedef struct AUDIO_ENGINE_t {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  float* scratchBuffer;
  size_t scratchBufferSize;
  // CHANNEL_LIST* pending;

  TABLE pending;
  TABLE channels;
  uintmax_t nextId;
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
  TABLE_ITERATOR iter;
  CHANNEL* channel;
  while (TABLE_iterate(&(audioEngine->channels), &iter)) {
    channel = iter.value;
    if (iter.done) {
      break;
    }

    assert(channel != NULL);
    size_t requestServed = 0;
    float* writeCursor = (float*)(stream);

    while (channel->enabled && requestServed < totalSamples) {
      SDL_memset(scratchBuffer, 0, bufferSampleSize * bytesPerSample);
      size_t requestSize = min(bufferSampleSize, totalSamples - requestServed);
      channel->methods.mix(channel, scratchBuffer, requestSize);
      requestServed += requestSize;
      float* copyCursor = scratchBuffer;
      float* endPoint = copyCursor + bufferSampleSize * channels;
      for (; copyCursor < endPoint; copyCursor++) {
        *(writeCursor++) += *copyCursor;
      }
    }
  }

  /*
  for (size_t c = 0; c < channelCount; c++) {
    CHANNEL* channel = (CHANNEL*)(playing->channels[c]);
    if (channel == NULL) {
      continue;
    }
    size_t requestServed = 0;
    float* writeCursor = (float*)(stream);

    while (channel->enabled && requestServed < totalSamples) {
      SDL_memset(scratchBuffer, 0, bufferSampleSize * bytesPerSample);
      size_t requestSize = min(bufferSampleSize, totalSamples - requestServed);
      channel->methods.mix(channel, scratchBuffer, requestSize);
      requestServed += requestSize;
      float* copyCursor = scratchBuffer;
      float* endPoint = copyCursor + bufferSampleSize * channels;
      for (; copyCursor < endPoint; copyCursor++) {
        *(writeCursor++) += *copyCursor;
      }
    }
  }
  */
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

  engine->scratchBuffer = calloc(AUDIO_BUFFER_SIZE, bytesPerSample);
  if (engine->scratchBuffer != NULL) {
    engine->scratchBufferSize = AUDIO_BUFFER_SIZE;
  }

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);

  TABLE* table = &(engine->channels);
  TABLE_init(table);

  return engine;
}

internal void
AUDIO_ENGINE_lock(AUDIO_ENGINE* engine) {
  SDL_LockAudioDevice(engine->deviceId);
}

internal void
AUDIO_ENGINE_unlock(AUDIO_ENGINE* engine) {
  SDL_UnlockAudioDevice(engine->deviceId);
}

/*
internal CHANNEL*
AUDIO_ENGINE_newChannel(AUDIO_ENGINE* engine) {
  size_t nextId = engine->nextId++;
  CHANNEL* channel = TABLE_reserve(&engine->channels, nextId);
  channel->id = nextId;
  return channel;
}
*/

internal uintmax_t
AUDIO_ENGINE_channelInit(
    AUDIO_ENGINE* engine,
    CHANNEL_mix mix,
    CHANNEL_callback update,
    CHANNEL_callback finish,
    void* userdata
    ) {

  uintmax_t id = engine->nextId++;
  CHANNEL channel;
  channel = {
    .state = CHANNEL_INITIALIZE,
    .mix = mix,
    .update = update,
    .finish = finish,
    .userdata = userdata
  }
  TABLE_set(&engine->pending, id, channel);

  return id;
}

internal void
AUDIO_ENGINE_update(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;
  TABLE_ITERATOR iter;




  /*
  CHANNEL_LIST* pending = data->pending;
  CHANNEL_LIST* playing = data->playing;

  // Copy enabled channels into pending
  size_t waitCount = pending->count;
  size_t next = 0;
  pending = CHANNEL_LIST_resize(pending, waitCount + playing->count);
  for (size_t i = 0; i < playing->count; i++) {
    CHANNEL* channel = (CHANNEL*)playing->channels[i];
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
    CHANNEL* channel = (CHANNEL*)data->playing->channels[i];
    assert(channel != NULL);
    if (channel->methods.update != NULL) {
      channel->methods.update(vm, channel);
    }
  }
  AUDIO_ENGINE_unlock(data);

  for (size_t i = 0; i < data->pending->count; i++) {
    CHANNEL* channel = (CHANNEL*)data->pending->channels[i];
    assert(channel != NULL);
    if (channel->enabled == false) {
      channel->enabled = false;
      if (channel->methods.finish != NULL) {
        channel->methods.finish(vm, channel);
      }
    }
  }
  data->pending = CHANNEL_LIST_resize(data->pending, 0);
  assert(data->pending->count == 0);

  // Safety - Make sure we don't misuse these pointers.
  pending = NULL;
  playing = NULL;
  */
}

internal void
AUDIO_ENGINE_stop(AUDIO_ENGINE* engine, uintmax_t id) {
  CHANNEL_LIST* playing = engine->playing;
  for (size_t i = 0; i < playing->count; i++) {
    CHANNEL* channel = (CHANNEL*)playing->channels[i];
    if (channel->id == id) {
      channel->stopRequested = true;
    }
  }
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
  free(engine->playing);
  // TODO: Free correctly the table and list
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
  list = realloc(list, sizeof(CHANNEL_LIST) + sizeof(CHANNEL) * channels);
  list->count = channels;
  memset(&(list->channels[current], 0, sizeof(CHANNEL) * max(0, channels - current));
  return list;
}
