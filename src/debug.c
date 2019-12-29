internal void DEBUG_printFloat(float f) {
  printf("%f\n", f);
}
internal void DEBUG_printInt(int i) {
  printf("%i\n", i);
}
internal void DEBUG_printString(char* str) {
  printf("%s\n", str);
}

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

internal void DEBUG_printAudioSpec(SDL_AudioSpec spec, AUDIO_TYPE type) {
  if (type == AUDIO_TYPE_WAV) {
    printf("WAV ");
  } else if (type == AUDIO_TYPE_WAV) {
    printf("OGG ");
  } else {
    printf("Unknown audio file detected\n");
  }
  printf("Audio: %i Hz ", spec.freq);
  printf("%s", spec.channels == 0 ? "Mono" : "Stereo");
  printf(" - ");
  if (SDL_AUDIO_ISSIGNED(spec.format)) {
    printf("Signed ");
  } else {
    printf("Unsigned ");
  }
  printf("%i bit (", SDL_AUDIO_BITSIZE(spec.format));
  if (SDL_AUDIO_ISLITTLEENDIAN(spec.format)) {
    printf("LSB");
  } else {
    printf("MSB");
  }
  printf(")\n");
}
