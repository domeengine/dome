internal CHANNEL_REF
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
AUDIO_API_stop(CHANNEL_REF ref) {
  AUDIO_ENGINE* engine = ref.engine;
  AUDIO_ENGINE_stop(engine, &ref);
}


internal void
AUDIO_API_setState(CHANNEL_REF ref, CHANNEL_STATE state) {
  AUDIO_ENGINE* engine = ref.engine;
  AUDIO_ENGINE_setState(engine, &ref, state);
}
internal CHANNEL_STATE
AUDIO_API_getState(CHANNEL_REF ref) {
  AUDIO_ENGINE* engine = ref.engine;
  return AUDIO_ENGINE_getState(engine, &ref);
}

internal void*
AUDIO_API_getData(CHANNEL_REF ref) {
  AUDIO_ENGINE* engine = ref.engine;
  return AUDIO_ENGINE_getData(engine, &ref);
}


AUDIO_API_v0 audio_v0 = {
  .channelCreate = AUDIO_API_channelCreate,
  .getState = AUDIO_API_getState,
  .setState = AUDIO_API_setState,
  .stop = AUDIO_API_stop,
  .getData = AUDIO_API_getData
};
