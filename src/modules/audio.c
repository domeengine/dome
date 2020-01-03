#define AUDIO_CHANNEL_START 2

typedef struct {
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
  size_t count;
  AUDIO_CHANNEL* channels[];
} AUDIO_CHANNEL_LIST;

typedef struct AUDIO_ENGINE_t {
  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec spec;
  AUDIO_CHANNEL_LIST* channelList;
} AUDIO_ENGINE;

const uint16_t channels = 2;
const uint16_t bytesPerSample = 2 * 2 /* channels */;

// audio callback function
// Allows SDL to "pull" data into the output buffer
// on a seperate thread. We need to be pretty efficient
// here as it holds a lock.
void AUDIO_ENGINE_mix(void*  userdata,
    Uint8* stream,
    int    outputBufferSize) {
  AUDIO_ENGINE* audioEngine = userdata;
  uint32_t totalSamples = outputBufferSize / bytesPerSample;

  int16_t* writeCursor = (int16_t*)(stream);
  SDL_memset(writeCursor, 0, outputBufferSize);

  int32_t samplesToWrite = totalSamples;//  - samplesQueued;
  int totalChannels = 0;

  // Get channel
  for (int i = 0; i < samplesToWrite; i++) {
    int totalEnabled = 0;
    float left = 0;
    float right = 0;
    AUDIO_CHANNEL_LIST* channelList = audioEngine->channelList;
    for (size_t c = 0; c < channelList->count; c++) {
      AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)(channelList->channels[c]);
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
}

internal void AUDIO_allocate(WrenVM* vm) {
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

    int channelsInFile = 0, freq = 0;
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
    data->spec.format = AUDIO_S16LSB;
  } else {
    VM_ABORT(vm, "Audio file was of an incompatible format");
    return;
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
  if (DEBUG_MODE) {
    DEBUG_printAudioSpec(data->spec, data->audioType);
  }
}

internal void AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    if (audioData->audioType == AUDIO_TYPE_WAV || audioData->audioType == AUDIO_TYPE_OGG) {
      free(audioData->buffer);
    }
    audioData->buffer = NULL;
  }
}
internal void AUDIO_unload(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "audio data");
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  AUDIO_finalize(data);
}

internal AUDIO_ENGINE*
AUDIO_ENGINE_init(void) {
  AUDIO_ENGINE* engine = malloc(sizeof(AUDIO_ENGINE));
  engine->channelList = malloc(sizeof(AUDIO_CHANNEL_LIST) + sizeof(AUDIO_CHANNEL*) * AUDIO_CHANNEL_START);
  engine->channelList->count = AUDIO_CHANNEL_START;
  for (int i = 0; i < AUDIO_CHANNEL_START; i++) {
    engine->channelList->channels[i] = NULL;
  }
  // SETUP player
  // set the callback function
  (engine->spec).freq = 44100;
  (engine->spec).format = AUDIO_S16LSB;
  (engine->spec).channels = channels; // TODO: consider mono/stereo
  (engine->spec).samples = AUDIO_BUFFER_SIZE; // Consider making this configurable
  (engine->spec).callback = AUDIO_ENGINE_mix;
  (engine->spec).userdata = engine;

  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, 0);
  // TODO: Handle if we can't get a device!

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);
  return engine;
}

internal AUDIO_CHANNEL_LIST*
AUDIO_CHANNEL_LIST_resize(AUDIO_CHANNEL_LIST* list, size_t channels) {
  if (list->count < channels) {
    list = realloc(list, sizeof(AUDIO_CHANNEL_LIST) + sizeof(AUDIO_CHANNEL*) * channels);
    list->count = channels;
    for (int i = 0; i < AUDIO_CHANNEL_START; i++) {
      list->channels[i] = NULL;
    }
  }
  return list;
}

internal void AUDIO_ENGINE_update(WrenVM* vm) {
  // We need additional slots to parse a list
  wrenEnsureSlots(vm, 3);
  ENGINE* engine = wrenGetUserData(vm);
  AUDIO_ENGINE* data = engine->audioEngine;

  SDL_LockAudioDevice(data->deviceId);
  ASSERT_SLOT_TYPE(vm, 1, LIST, "channels");
  uint8_t soundCount = wrenGetListCount(vm, 1);
  data->channelList = AUDIO_CHANNEL_LIST_resize(data->channelList, soundCount);
  for (size_t i = 0; i < data->channelList->count; i++) {
    if (i < soundCount) {
      wrenGetListElement(vm, 1, i, 2);
      if (wrenGetSlotType(vm, 2) != WREN_TYPE_NULL) {
        data->channelList->channels[i] = wrenGetSlotForeign(vm, 2);
      }
    } else {
      data->channelList->channels[i] = NULL;
    }
  }
  SDL_UnlockAudioDevice(data->deviceId);
}

internal void AUDIO_ENGINE_halt(AUDIO_ENGINE* engine) {
  if (engine != NULL) {
    SDL_PauseAudioDevice(engine->deviceId, 1);
    SDL_CloseAudioDevice(engine->deviceId);
  }
}

internal void AUDIO_ENGINE_free(AUDIO_ENGINE* engine) {
  // We might need to free contained audio here
  AUDIO_ENGINE_halt(engine);
  free(engine->channelList);
}

internal void AUDIO_CHANNEL_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  ASSERT_SLOT_TYPE(vm, 1, NUM, "channel id");
  int16_t id = (int16_t)wrenGetSlotDouble(vm, 1);
  data->channelId = id;
  data->enabled = true;
  data->loop = false;
  ASSERT_SLOT_TYPE(vm, 2, FOREIGN, "audio");
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
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "enabled");
  channel->enabled = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_setLoop(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "loop");
  channel->loop = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_setVolume(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "volume");
  channel->volume = (float)max(0, wrenGetSlotDouble(vm, 1));
}

internal void AUDIO_CHANNEL_setPan(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "pan");
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
