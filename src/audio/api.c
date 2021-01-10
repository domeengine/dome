
internal AUDIO_CHANNEL_REF
AUDIO_API_channelCreate(
    DOME_Context ctx,
    CHANNEL_mix mix,
    CHANNEL_callback update,
    CHANNEL_callback finish,
    void* userdata) {

  AUDIO_CHANNEL_REF result;
  AUDIO_ENGINE* engine = ((ENGINE*)ctx)->audioEngine;
  result.id = AUDIO_ENGINE_channelInit(
      engine,
      mix,
      update,
      finish,
      userdata
  );
  return result;
 }

internal void
AUDIO_API_stop(DOME_Context ctx, AUDIO_CHANNEL_REF ref) {
  AUDIO_ENGINE* engine = ((ENGINE*)ctx)->audioEngine;
  AUDIO_ENGINE_stop(engine, &ref);
}

AUDIO_API_v0 audio_v0 = {
  .channelCreate = AUDIO_API_channelCreate,
  .stop = AUDIO_API_stop
};
