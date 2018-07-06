typedef struct {
  int32_t width;
  int32_t height;
  int32_t channels;
  uint32_t* pixels;
} IMAGE;

void IMAGE_allocate(WrenVM* vm) {

  // TODO: Get the image file name
  // read in the file
  // convert to the correct pixel format
  const char* path = wrenGetSlotString(vm, 1);
  IMAGE* image = (IMAGE*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(IMAGE));
  image->pixels = (uint32_t*)stbi_load(path,
      &image->width,
      &image->height,
      &image->channels,
      STBI_rgb_alpha);

  if (image->pixels == NULL)
  {
    wrenSetSlotString(vm, 0, "A problem loading the file");
    wrenAbortFiber(vm, 0);
    return;
  } else {

    printf("Image loaded: %s\n", path);

  }
  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int i = 0; i < image->height * image->width; i++) {
    uint32_t c = *pixel;
    uint8_t r = (0x000000FF & c);
    uint8_t g = (0x0000FF00 & c) >> 8;
    uint8_t b = (0x00FF0000 & c) >> 16;
    uint8_t a = (0xFF000000 & c) << 24;
    *pixel = a | (r << 16) | (g << 8) | b;
    pixel++;
  }
}

void IMAGE_finalize(void* data) {
  IMAGE* image = data;

  if (image->pixels != NULL) {
    stbi_image_free(image->pixels);
  }

}

void IMAGE_draw(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  uint32_t x = wrenGetSlotDouble(vm, 1);
  uint32_t y = wrenGetSlotDouble(vm, 2);

  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      uint32_t c = pixel[j * image->width + i];
      ENGINE_pset(engine, x+i, y+j, c);
    }
  }
}
