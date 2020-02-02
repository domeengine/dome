
internal char*
DEBUG_printWrenType(WrenType type) {
  switch (type) {
    case WREN_TYPE_BOOL: return "boolean"; break;
    case WREN_TYPE_NUM: return "number"; break;
    case WREN_TYPE_FOREIGN: return "foreign"; break;
    case WREN_TYPE_LIST: return "list"; break;
    case WREN_TYPE_NULL: return "null"; break;
    case WREN_TYPE_STRING: return "string"; break;
    default: return "unknown";
  }
}

// forward declare
internal void ENGINE_printLog(ENGINE* engine, char* line, ...);
#define eprintf(...) ENGINE_printLog(engine, __VA_ARGS__)

internal void DEBUG_printAudioSpec(ENGINE* engine, SDL_AudioSpec spec, AUDIO_TYPE type) {
  if (type == AUDIO_TYPE_WAV) {
    eprintf("WAV ");
  } else if (type == AUDIO_TYPE_OGG) {
    eprintf("OGG ");
  } else {
    eprintf("Unknown audio file detected\n");
  }
  eprintf("Audio: %i Hz ", spec.freq);
  eprintf("%s", spec.channels == 0 ? "Mono" : "Stereo");
  eprintf(" - ");
  if (SDL_AUDIO_ISSIGNED(spec.format)) {
    eprintf("Signed ");
  } else {
    eprintf("Unsigned ");
  }
  eprintf("%i bit (", SDL_AUDIO_BITSIZE(spec.format));
  if (SDL_AUDIO_ISLITTLEENDIAN(spec.format)) {
    eprintf("LSB");
  } else {
    eprintf("MSB");
  }
  eprintf(")\n");
}
#undef eprintf
