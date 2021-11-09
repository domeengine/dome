internal void*
IO_API_readFile(DOME_Context ctx, const char* path, size_t* lengthPtr) {
  ENGINE* engine = (ENGINE*)ctx;
  return ENGINE_readFile(engine, path, lengthPtr);
}

internal DOME_Bitmap*
IO_API_imageFromBuffer(DOME_Context ctx, void* buffer, size_t length) {
  DOME_Bitmap* bitmap = malloc(sizeof(DOME_Bitmap));
  bitmap->pixels = (DOME_Color*)stbi_load_from_memory((const stbi_uc*)buffer, length,
      &bitmap->width, &bitmap->height,
      &bitmap->channels, STBI_rgb_alpha);
  // TODO: handle errors
  return bitmap;
}

internal DOME_Bitmap*
IO_API_readImageFile(DOME_Context ctx, const char* path) {
  size_t length;
  void* buffer = IO_API_readFile(ctx, path, &length);
  return IO_API_imageFromBuffer(ctx, buffer, length);
}

internal void
IO_API_freeBitmap(DOME_Bitmap* bitmap) {
  if (bitmap->pixels != NULL) {
    free(bitmap->pixels);
  }
  free(bitmap);
}

IO_API_v0 io_v0 = {
  .readFile = IO_API_readFile,
  .imageFromBuffer = IO_API_imageFromBuffer,
  .readImageFile = IO_API_readImageFile,
  .freeBitmap = IO_API_freeBitmap
};
