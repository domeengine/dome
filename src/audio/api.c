
internal AUDIO_CHANNEL_REF
AUDIO_API_channelCreate(
    DOME_Context ctx,
    CHANNEL_mix mix,
    CHANNEL_callback update,
    CHANNEL_callback finish,
    void* userdata) {

  AUDIO_ENGINE* engine = ((ENGINE*)ctx)->audioEngine;
  return AUDIO_ENGINE_channelInit(
      engine,
      mix,
      update,
      finish,
      userdata
  );
 }

internal void
AUDIO_API_stop(DOME_Context ctx, AUDIO_CHANNEL_REF ref) {
  AUDIO_ENGINE* engine = ((ENGINE*)ctx)->audioEngine;
  AUDIO_ENGINE_stop(engine, &ref);
}


internal void
AUDIO_API_setState(DOME_Context ctx, AUDIO_CHANNEL_REF ref, CHANNEL_STATE state) {
  AUDIO_ENGINE* engine = ((ENGINE*)ctx)->audioEngine;
  AUDIO_ENGINE_setState(engine, &ref, state);
}

AUDIO_API_v0 audio_v0 = {
  .channelCreate = AUDIO_API_channelCreate,
  .setState = AUDIO_API_setState,
  .stop = AUDIO_API_stop
};
