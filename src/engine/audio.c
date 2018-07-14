
typedef struct {
  SDL_AudioSpec spec;
  uint32_t length;
  char name[256];
  uint8_t* buffer;
} AUDIO_DATA;

typedef struct {
  uint32_t channelId;
  uint32_t position;
  AUDIO_DATA* audio;
}

typedef struct {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  AUDIO_DATA* playing;
  bool enabled;
  int16_t audioScale;
} AUDIO_ENGINE;

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void AUDIO_ENGINE_callback(void *userData, uint8_t *stream, int len) {

  AUDIO_ENGINE* audioEngine = (AUDIO_ENGINE*)userData;
  if (audioEngine->playing != NULL) {


  }

  // We need to cast the pointer to the actual type for our audio buffer.
  int16_t *buf = (int16_t*)stream;
  // And account for the fact that a larger type means a "smaller" buffer.
  len = len / 2;

  // Copy and perform DSP here
  int16_t audioScale = audioEngine->audioScale;
  for (int i = 0; i < len/2; i++) {
     if (audioEngine->playing == NULL) {
       buf[i*2] = 0;
       buf[i*2+1] = 0;
     } else {
       buf[i*2] = audioEngine->playing->buffer[i*2];
       buf[i*2+1] = audioEngine->playing->buffer[i*2+1];

     }
    // buf[i*2] = (i % 2 == 0) ? (1<<audioScale) : -(1<<audioScale);
    // buf[i*2 + 1] = (i % 2 == 0) ? (1<<audioScale) : -(1<<audioScale);
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
  uint8_t soundCount = wrenGetListCount(vm, 1);
  data->playing = NULL;
  for (int i = 0; i < soundCount; i++) {
    wrenGetListElement(vm, 1, i, 2);
    data->playing = wrenGetSlotForeign(vm, 2);
  }
  SDL_LockAudioDevice(data->deviceId);
  data->audioScale--;
  if (data->audioScale < 0) {
    data->audioScale = 14;
  }
  SDL_UnlockAudioDevice(data->deviceId);
}

internal void AUDIO_ENGINE_finalize(void* audioEngine) {
  // We might need to free contained audio here
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)audioEngine;
  SDL_PauseAudioDevice(engine->deviceId, 1);
  SDL_CloseAudioDevice(engine->deviceId);
}
/*
internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}


internal int AUDIO_ENGINE_play_sound(char* soundName, double volume) {}
internal void AUDIO_ENGINE_stop_channel(uint16_t channelId) {}
internal void AUDIO_ENGINE_stop_all_channels() {}
internal void AUDIO_ENGINE_set_channel_volume(uint16_t channelId, double volume) {}
internal bool AUDIO_ENGINE_is_playing(uint16_t channelId) {}
*/
