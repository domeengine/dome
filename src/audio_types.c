typedef enum {
  AUDIO_TYPE_UNKNOWN,
  AUDIO_TYPE_WAV,
  AUDIO_TYPE_OGG
} AUDIO_TYPE;

typedef enum {
  CHANNEL_INVALID,
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_DEVIRTUALIZE,
  CHANNEL_LOADING,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED,
  CHANNEL_VIRTUALIZING,
  CHANNEL_VIRTUAL,
  CHANNEL_LAST
} CHANNEL_STATE;

typedef void (*CHANNEL_mix)(void* channel, float* buffer, size_t requestedSamples);
typedef void (*CHANNEL_callback)(WrenVM* vm, void* channel);

typedef struct {
  CHANNEL_mix mix;
  CHANNEL_callback update;
  CHANNEL_callback finish;
} CHANNEL_METHODS;

typedef struct {
  CHANNEL_STATE state;
  uintmax_t id;
  volatile bool enabled;
  bool stopRequested;
  CHANNEL_METHODS methods;

  void* userdata;
  WrenHandle* handle;
} CHANNEL;


typedef struct {
  size_t count;
  CHANNEL channels[];
} CHANNEL_LIST;

