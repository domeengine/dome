#define AUDIO_CHANNEL_MAX 8

#define AUDIO_TYPE_UNKNOWN 0
#define AUDIO_TYPE_WAV 1
#define AUDIO_TYPE_OGG 2

typedef uint8_t AUDIO_TYPE;

typedef struct {
  char name[256];
  SDL_AudioSpec spec;
  AUDIO_TYPE audioType;
  // Length is the number of LR samples
  uint32_t length;
  // Audio is stored as a stream of interleaved normalised values from [-1, 1)
  float* buffer;
} AUDIO_DATA;

typedef struct {
  int16_t channelId;
  // Control variables
  bool enabled;
  bool loop;

  // Playback variables
  float volume;
  float pan;

  // Position is the sample value to play next
  uint32_t position;
  AUDIO_DATA* audio;
} AUDIO_CHANNEL;

typedef struct {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  int16_t audioScale;
  AUDIO_CHANNEL* channels[AUDIO_CHANNEL_MAX];
  uint8_t* outputBuffer;
} AUDIO_ENGINE;

const uint16_t channels = 2;
const uint16_t bytesPerSample = 2 * 2 /* channels */;

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void AUDIO_ENGINE_mix(AUDIO_ENGINE* audioEngine) {
  uint32_t totalSamples = audioEngine->spec.samples;
  uint32_t outputBufferSize = totalSamples * bytesPerSample;

  int16_t* writeCursor = (int16_t*)(audioEngine->outputBuffer);
  SDL_memset(writeCursor, 0, outputBufferSize);

  int32_t samplesQueued = SDL_GetQueuedAudioSize(audioEngine->deviceId) / bytesPerSample;
  int32_t samplesToWrite = totalSamples - samplesQueued;
  int totalChannels = 0;

  // Get channel
  for (int i = 0; i < samplesToWrite; i++) {
    int totalEnabled = 0;
    float left = 0;
    float right = 0;
    for (int c = 0; c < AUDIO_CHANNEL_MAX; c++) {
      AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)(audioEngine->channels[c]);
      if (channel != NULL) {
        AUDIO_DATA* audio = channel->audio;
        if (channel->enabled) {
          totalEnabled++;
          float* readCursor = (float*)(audio->buffer);
          readCursor += channel->position * channels;
          float volume = channel->volume;
          float pan = (channel->pan + 1) * M_PI / 4; // Channel pan is [-1,1] real pan needs to be [0,1]

          left += readCursor[0] * cos(pan) * volume;
          right += readCursor[1] * sin(pan) * volume;
        }
        channel->position++;
        if (channel->loop && channel->position >= audio->length) {
          channel->position = 0;
        }
        channel->enabled = channel->enabled && channel->position < audio->length;
      }
    }
    if (totalEnabled > 1) {
      left = (float)tanh(left); ///= totalEnabled;
      right = (float)tanh(right); //= totalEnabled;
    }
    if (totalEnabled > 0) {
      writeCursor[i*2] = (int16_t)(left * INT16_MAX);
      writeCursor[i*2+1] = (int16_t)(right * INT16_MAX);
    }
    totalChannels = max(totalEnabled, totalChannels);
  }
  SDL_QueueAudio(audioEngine->deviceId, audioEngine->outputBuffer, samplesToWrite*bytesPerSample);
}

internal void AUDIO_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  AUDIO_DATA* data = (AUDIO_DATA*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_DATA));
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* path = wrenGetSlotString(vm, 1);

  /*
  char* base = SDL_GetBasePath();
  char pathBuf[strlen(base)+strlen(path)+1];
  strcpy(pathBuf, base);
  strcat(pathBuf, path);
  SDL_free(base);
  */

  strncpy(data->name, path, 255);
  data->name[255] = '\0';

  // make internal file name lowercase
  for(int i = 0; data->name[i]; i++){
    data->name[i] = tolower(data->name[i]);
  }

  int16_t* tempBuffer;
  size_t length;
  uint8_t* fileBuffer = (uint8_t*)ENGINE_readFile(engine, path, &length);

  if (strstr(data->name, ".wav") != NULL) {
    data->audioType = AUDIO_TYPE_WAV;

    // Loading the WAV file
    SDL_RWops* src = SDL_RWFromConstMem(fileBuffer, length);
    SDL_LoadWAV_RW(src, 1, &data->spec, ((uint8_t**)&tempBuffer), &data->length);
    // SDL_LoadWAV(pathBuf, &data->spec, ((uint8_t**)&tempBuffer), &data->length);
    data->length /= sizeof(int16_t) * data->spec.channels;
  } else if (strstr(data->name, ".ogg") != NULL) {
    data->audioType = AUDIO_TYPE_OGG;

    int channelsInFile = 0, freq = 0;
    memset(&data->spec, 0, sizeof(SDL_AudioSpec));
    // Loading the OGG file
    data->length = stb_vorbis_decode_memory(fileBuffer, length, &channelsInFile, &freq, &tempBuffer);

    // data->length = stb_vorbis_decode_filename(pathBuf, &channelsInFile, &freq, &tempBuffer);
    data->spec.channels = channelsInFile;
    data->spec.freq = freq;
    data->spec.format = AUDIO_S16LSB;
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
  DEBUG_printAudioSpec(data->spec);
  printf("Audio loaded: %s\n", path);
  free(fileBuffer);
}

internal void AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    if (audioData->audioType == AUDIO_TYPE_WAV || audioData->audioType == AUDIO_TYPE_OGG) {
      free(audioData->buffer);
    }
    audioData->buffer = NULL;
    printf("Audio unloaded: %s\n", audioData->name);
  }
}
internal void AUDIO_unload(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  AUDIO_finalize(data);
}

internal void AUDIO_ENGINE_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_ENGINE));
  engine->audioScale = 15;
  for (int i = 0; i < AUDIO_CHANNEL_MAX; i++) {
    engine->channels[i] = NULL;
  }
  // SETUP player
  // set the callback function
  (engine->spec).freq = 44100;
  (engine->spec).format = AUDIO_S16LSB;
  (engine->spec).channels = channels; // TODO: consider mono/stereo
  (engine->spec).samples = 4096;
  (engine->spec).callback = NULL;
  (engine->spec).userdata = engine;

  engine->outputBuffer = calloc(engine->spec.samples * channels * bytesPerSample, sizeof(uint8_t));

  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, 0);
  // TODO: Handle if we can't get a device!

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);
}

internal void AUDIO_ENGINE_update(WrenVM* vm) {
  // We need additional slots to parse a list
  wrenEnsureSlots(vm, 3);
  AUDIO_ENGINE* data = (AUDIO_ENGINE*)wrenGetSlotForeign(vm, 0);
  uint8_t soundCount = wrenGetListCount(vm, 1);
  for (int i = 0; i < AUDIO_CHANNEL_MAX; i++) {
    if (i < soundCount) {
      wrenGetListElement(vm, 1, i, 2);
      if (wrenGetSlotType(vm, 2) != WREN_TYPE_NULL) {
        data->channels[i] = wrenGetSlotForeign(vm, 2);
      }
    } else {
      data->channels[i] = NULL;
    }
  }
  AUDIO_ENGINE_mix(data);
}

internal void AUDIO_ENGINE_finalize(void* audioEngine) {
  // We might need to free contained audio here
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)audioEngine;
  SDL_PauseAudioDevice(engine->deviceId, 1);
  SDL_CloseAudioDevice(engine->deviceId);
  free(engine->outputBuffer);
}

internal void AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  int16_t id = (int16_t)wrenGetSlotDouble(vm, 1);
  data->channelId = id;
  data->enabled = true;
  data->loop = false;
  data->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 2);
}

internal void AUDIO_CHANNEL_isFinished(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, !data->enabled);
}

internal void AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, data->channelId);
}

internal void AUDIO_CHANNEL_setEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->enabled = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_setLoop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->loop = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->volume = (float)max(0, wrenGetSlotDouble(vm, 1));
}

internal void AUDIO_CHANNEL_setPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->pan = (float)mid(-1.0, wrenGetSlotDouble(vm, 1), 1.0f);
}

internal void AUDIO_CHANNEL_finalize(void* data) {
  // printf("Channel finished\n");
}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
