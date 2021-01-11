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

internal void
AUDIO_ENGINE_setPosition(AUDIO_ENGINE* engine, AUDIO_CHANNEL_REF* ref, size_t position) {
  CHANNEL* base;
  if (!AUDIO_ENGINE_get(engine, ref, &base)) {
    return;
  }
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  channel->new.position = mid(0, position, channel->audio->length);
  channel->new.resetPosition = true;
}

internal void
AUDIO_ENGINE_setState(AUDIO_ENGINE* engine, AUDIO_CHANNEL_REF* ref, CHANNEL_STATE state) {
  CHANNEL* base;
  if (!AUDIO_ENGINE_get(engine, ref, &base)) {
    return;
  }
  CHANNEL_setState(base, state);
}


#define AUDIO_CHANNEL_GETTER(fieldName, method, fieldType, defaultValue) \
  internal fieldType \
  AUDIO_ENGINE_get##method(AUDIO_ENGINE* engine, AUDIO_CHANNEL_REF* ref) { \
    CHANNEL* base; \
    if (!AUDIO_ENGINE_get(engine, ref, &base)) { \
      return defaultValue; \
    } \
    AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base); \
    return channel->fieldName; \
  }

#define AUDIO_CHANNEL_SETTER(fieldName, method, fieldType) \
  internal void \
  AUDIO_ENGINE_set##method(AUDIO_ENGINE* engine, AUDIO_CHANNEL_REF* ref, fieldType value) { \
    CHANNEL* base; \
    if (!AUDIO_ENGINE_get(engine, ref, &base)) { \
      return; \
    } \
    AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base); \
    channel->fieldName = value; \
  }
#define AUDIO_CHANNEL_SETTER_WITH_RANGE(fieldName, method, fieldType, min, max) \
  internal void \
  AUDIO_ENGINE_set##method(AUDIO_ENGINE* engine, AUDIO_CHANNEL_REF* ref, fieldType value) { \
    CHANNEL* base; \
    if (!AUDIO_ENGINE_get(engine, ref, &base)) { \
      return; \
    } \
    AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base); \
    channel->fieldName = fmid(min, value, max); \
  }

// volume
AUDIO_CHANNEL_SETTER_WITH_RANGE(new.volume, Volume, float, 0.0f, 1.0f)
AUDIO_CHANNEL_GETTER(new.volume, Volume, float, 0.0f)
// pan
AUDIO_CHANNEL_SETTER_WITH_RANGE(new.pan, Pan, float, -1.0f, 1.0f)
AUDIO_CHANNEL_GETTER(new.pan, Pan, float, 0.0f)

// loop
AUDIO_CHANNEL_SETTER(new.loop, Loop, bool)
AUDIO_CHANNEL_GETTER(new.loop, Loop, bool, false)
// position
AUDIO_CHANNEL_GETTER(new.position, Position, size_t, 0)


internal void
AUDIO_CHANNEL_finish(WrenVM* vm, CHANNEL* base) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)CHANNEL_getData(base);
  assert(channel != NULL);
  if (channel->audioHandle != NULL) {
    wrenReleaseHandle(vm, channel->audioHandle);
    channel->audioHandle = NULL;
  }
  free(channel->soundId);
  free(channel);
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
      if (CHANNEL_getEnabled(base) == false || CHANNEL_hasStopRequested(base)) {
        CHANNEL_setState(base, CHANNEL_STOPPING);
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
    float currentVolume = actualVolume;
    if (actualVolume != volume) {
      currentVolume = lerp(actualVolume, volume, f);
    }
    float currentPan = actualPan;
    if (actualPan != targetPan) {
      currentPan = lerp(actualPan, targetPan, f);
    }
    float pan = (currentPan + 1.0f) * M_PI / 4.0f; // Channel pan is [-1,1] real pan needs to be [0,1]

    // We have to advance the cursor after each read and write
    // Read/Write left
    *(writeCursor++) = *(readCursor++) * cos(pan) * currentVolume;
    // Read/Write right
    *(writeCursor++) = *(readCursor++) * sin(pan) * currentVolume;

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

internal AUDIO_CHANNEL_REF
AUDIO_CHANNEL_new(AUDIO_ENGINE* engine, char* soundId) {

  AUDIO_CHANNEL* data = malloc(sizeof(AUDIO_CHANNEL));
  data->soundId = strdup(soundId);
  struct AUDIO_CHANNEL_PROPS props = {0, 0, 0, 0, 0};
  data->current = data->new = props;
  data->actualVolume = 0.0f;
  data->actualPan = 0.0f;
  data->audio = NULL;

  CHANNEL_ID id = AUDIO_ENGINE_channelInit(
    engine,
    AUDIO_CHANNEL_mix,
    AUDIO_CHANNEL_update,
    AUDIO_CHANNEL_finish,
    data
  );
  AUDIO_CHANNEL_REF ref = {
    .id = id
  };
  return ref;
}

