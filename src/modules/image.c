typedef struct {
  int32_t width;
  int32_t height;
  int32_t channels;
  uint32_t* pixels;
} IMAGE;

typedef enum { COLOR_MODE_RGBA, COLOR_MODE_MONO } COLOR_MODE;

typedef struct {
  IMAGE* image;
  bool flipVertical;
  bool flipHorizontal;
  bool rotate;

  int32_t sourceX;
  int32_t sourceY;
  int32_t sourceWidth;
  int32_t sourceHeight;

  int32_t destX;
  int32_t destY;

  COLOR_MODE mode;
  // MONO colour palette
  uint32_t background;
  uint32_t foreground;
} DRAW_COMMAND;

void IMAGE_allocate(WrenVM* vm) {

  ASSERT_SLOT_TYPE(vm, 1, STRING, "image");
  // TODO: We should read this from a "DataBuffer" which is file loaded, rather than loading ourselves.
  // So that we can defer the file loading to a thread.
  int length;
  const char* fileBuffer = wrenGetSlotBytes(vm, 1, &length);
  IMAGE* image = (IMAGE*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(IMAGE));

  image->pixels = (uint32_t*)stbi_load_from_memory((const stbi_uc*)fileBuffer, length,
      &image->width,
      &image->height,
      &image->channels,
      STBI_rgb_alpha);

  if (image->pixels == NULL)
  {
    char* errorMsg = stbi_failure_reason();
    size_t errorLength = strlen(errorMsg);
    char buf[errorLength + 8];
    snprintf(buf, errorLength + 8, "Error: %s\n", errorMsg);
    wrenSetSlotString(vm, 0, buf);
    wrenAbortFiber(vm, 0);
    return;
  }
  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int i = 0; i < image->height * image->width; i++) {
    uint32_t c = *pixel;

    uint8_t r = (0x000000FF & c);
    uint8_t g = (0x0000FF00 & c) >> 8;
    uint8_t b = (0x00FF0000 & c) >> 16;
    uint8_t a = (0xFF000000 & c) >> 24;
    *pixel = (a << 24) | (r << 16) | (g << 8) | b;
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
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  uint32_t x = wrenGetSlotDouble(vm, 1);
  uint32_t y = wrenGetSlotDouble(vm, 2);

  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int32_t j = 0; j < image->height; j++) {
    for (int32_t i = 0; i < image->width; i++) {


      uint32_t c = pixel[j * image->width + i];
      ENGINE_pset(engine, x+i, y+j, c);
    }
  }
}

void IMAGE_getWidth(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, image->width);
}
void IMAGE_getHeight(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, image->height);
}

void IMAGE_drawArea(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "source x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "source y");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "source width");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "source height");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "destination x");
  ASSERT_SLOT_TYPE(vm, 6, NUM, "destination y");
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  int32_t srcX = wrenGetSlotDouble(vm, 1);
  int32_t srcY = wrenGetSlotDouble(vm, 2);
  int32_t srcW = wrenGetSlotDouble(vm, 3);
  int32_t srcH = wrenGetSlotDouble(vm, 4);
  int32_t destX = wrenGetSlotDouble(vm, 5);
  int32_t destY = wrenGetSlotDouble(vm, 6);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  double areaHeight = mid(0, srcH, image->height);
  double areaWidth = mid(0, srcW, image->width);

  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int32_t j = 0; j < areaHeight; j++) {
    for (int32_t i = 0; i < areaWidth; i++) {
      uint32_t c = pixel[(srcY+j) * image->width + (srcX+i)];
      ENGINE_pset(engine, destX+i, destY+j, c);
    }
  }
}
