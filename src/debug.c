internal void DEBUG_printFloat(float f) {
  printf("%f\n", f);
}
internal void DEBUG_printInt(int i) {
  printf("%i\n", i);
}
internal void DEBUG_printString(char* str) {
  printf("%s\n", str);
}

internal void DEBUG_printAudioSpec(SDL_AudioSpec spec) {
  printf("Frequency: %i Hz\n", spec.freq);
  printf("Samples: %i\n", spec.samples);
  printf("Channels: %i\n", spec.channels);
  printf("Format: ");
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
