
typedef struct {
  SDL_AudioSpec spec;
  uint32_t length;
  char name[256];
  uint8_t* buffer;
} AUDIO_DATA;

typedef struct {
  bool enabled;
  int16_t channelId;
  uint32_t position;
  uint8_t volume;
  AUDIO_DATA* audio;
} AUDIO_CHANNEL;

typedef struct {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  int16_t audioScale;
  AUDIO_CHANNEL* channels[4];
} AUDIO_ENGINE;

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void AUDIO_ENGINE_callback(void *userData, uint8_t *stream, int len) {

  AUDIO_ENGINE* audioEngine = (AUDIO_ENGINE*)userData;
  // We need to cast the pointer to the actual type for our audio buffer.
  int16_t *buf = (int16_t*)stream;
  // And account for the fact that a larger type means a "smaller" buffer.
  len = len / 2;

  // Get channel
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)(audioEngine->channels[0]);
  if (channel != NULL && channel->enabled) {
    int16_t* channelBuffer = (int16_t*)(channel->audio->buffer);
    channelBuffer += channel->position;

    // Copy and perform DSP here
    for (int i = 0; i < len/2; i++) {
      buf[i*2] = channelBuffer[i*2];
      buf[i*2+1] = channelBuffer[i*2+1];
    }
    channel->position += len*2;
    if (channel->position >= channel->audio->length / 2) {
      channel->enabled = false;
    }
  } else {
    // Write silence
    for (int i = 0; i < len/2; i++) {
      buf[i*2] = 0;
      buf[i*2+1] = 0;
    }
  }
}

internal void AUDIO_allocate(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_DATA));
  const char* path = wrenGetSlotString(vm, 1);
  strncpy(data->name, path, 255);
  data->name[255] = '\0';
  SDL_LoadWAV(path, &data->spec, &data->buffer, &data->length);
  printf("Audio loaded: %s\n", path);
}

internal void AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    SDL_FreeWAV(audioData->buffer);
    audioData->buffer = NULL;
    printf("Audio unloaded: %s\n", audioData->name);
  }
}
internal void AUDIO_unload(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  AUDIO_finalize(data);
}

internal void AUDIO_ENGINE_allocate(WrenVM* vm) {
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_ENGINE));
  engine->audioScale = 15;
  for (int i = 0; i < 4; i++) {
    engine->channels[i] = NULL;
  }
  // SETUP player
  // set the callback function
  (engine->spec).freq = 48000;
  (engine->spec).format = AUDIO_S16LSB;
  (engine->spec).channels = 2; // TODO: consider mono/stereo
  (engine->spec).samples = 4096;
  (engine->spec).callback = AUDIO_ENGINE_callback;
  (engine->spec).userdata = engine;


  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
  // TODO: Handle if we can't get a device!

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);
}

internal void AUDIO_ENGINE_update(WrenVM* vm) {
  // We need additional slots to parse a list
  wrenEnsureSlots(vm, 3);
  AUDIO_ENGINE* data = (AUDIO_ENGINE*)wrenGetSlotForeign(vm, 0);
  SDL_LockAudioDevice(data->deviceId);
  uint8_t soundCount = wrenGetListCount(vm, 1);
  //printf("Count %i\n", soundCount);
  for (int i = 0; i < 4; i++) {
    if (i < soundCount) {
      wrenGetListElement(vm, 1, i, 2);
      if (wrenGetSlotType(vm, 2) != WREN_TYPE_NULL) {
        data->channels[i] = wrenGetSlotForeign(vm, 2);
      }
    } else {
      data->channels[i] = NULL;
    }
  }
  SDL_UnlockAudioDevice(data->deviceId);
}

internal void AUDIO_ENGINE_finalize(void* audioEngine) {
  // We might need to free contained audio here
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)audioEngine;
  SDL_LockAudioDevice(engine->deviceId);
  SDL_PauseAudioDevice(engine->deviceId, 1);
  SDL_CloseAudioDevice(engine->deviceId);
  SDL_UnlockAudioDevice(engine->deviceId);
}

internal void AUDIO_CHANNEL_allocate(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  int16_t id = (int16_t)wrenGetSlotDouble(vm, 1);
  data->channelId = id;
  data->enabled = true;
  data->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 2);
}

internal void AUDIO_CHANNEL_isFinished(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, !data->enabled);
}

internal void AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, data->channelId);
}

internal void AUDIO_CHANNEL_setEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->enabled = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_finalize(void* data) {
  printf("Channel finished\n");
}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
