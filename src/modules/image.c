typedef struct {
  int32_t width;
  int32_t height;
  uint32_t* pixels;
  int32_t channels;
} IMAGE;

typedef enum { COLOR_MODE_RGBA, COLOR_MODE_MONO } COLOR_MODE;

typedef struct {
  IMAGE* image;
  VEC scale;
  double angle;

  int32_t srcW;
  int32_t srcH;

  iVEC src;
  VEC dest;

  COLOR_MODE mode;
  // MONO colour palette
  uint32_t backgroundColor;
  uint32_t foregroundColor;
} DRAW_COMMAND;

DRAW_COMMAND DRAW_COMMAND_init(IMAGE* image) {
  DRAW_COMMAND command;
  command.image = image;

  command.src = (iVEC) { 0, 0 };
  command.srcW = image->width;
  command.srcH = image->height;
  command.dest = (VEC) { 0, 0 };

  command.angle = 0;
  command.scale = (VEC) { 1, 1 };

  command.mode = COLOR_MODE_RGBA;
  command.backgroundColor = 0xFF000000;
  command.foregroundColor = 0xFFFFFFFF;

  return command;
}

internal void
DRAW_COMMAND_execute(ENGINE* engine, DRAW_COMMAND* commandPtr) {

  DRAW_COMMAND command = *commandPtr;
  IMAGE* image = command.image;

  iVEC src = command.src;
  int32_t srcW = command.srcW;
  int32_t srcH = command.srcH;

  VEC dest = command.dest;

  double angle = command.angle;
  VEC scale = command.scale;

  uint32_t* pixel = (uint32_t*)image->pixels;
  pixel = pixel + (src.y * image->width + src.x);


  int direction = round(angle / 90);
  direction %= 4;
  if(direction < 0) direction += 4;

  double sX = (1.0 / scale.x);
  double sY = (1.0 / scale.y);

  if (direction >= 0) {
    double swap;
    sX = fabs(sX);
    sY = fabs(sY);

    int32_t w = (srcW) * fabs(scale.x);
    int32_t h = (srcH) * fabs(scale.y);

    if (direction & 1) {
      swap = w;
      w = h;
      h = swap;
    }
    for (int32_t j = 0; j < ceil(h); j++) {
      for (int32_t i = 0; i < ceil(w); i++) {

        int32_t x = dest.x + i;
        int32_t y = dest.y + j;

        double q = i * sX;
        double t = j * sY;

        int32_t u = (q);
        int32_t v = (t);

        if ((scale.y > 0 && direction >= 2) || (scale.y < 0 && direction < 2)) {
          y = dest.y + (h-1) - j;
        }

        bool flipX = ((direction == 1 || direction == 2));
        if ((scale.x < 0 && !flipX) || (scale.x > 0 && flipX)) {
          x = dest.x + (w-1) - i;
        }

        if (direction & 1) {
          swap = u;
          u = v;
          v = swap;
        }

        uint32_t color = *(pixel + (v * image->width + u));
        if (command.mode == COLOR_MODE_MONO) {
          uint8_t alpha = (0xFF000000 & color) >> 24;
          if (alpha < 0xFF || (color & 0x00FFFFFF) == 0) {
            color = command.backgroundColor;
          } else {
            color = command.foregroundColor;
          }
        }
        ENGINE_pset(engine, x, y, color);
      }
    }
  }
}


internal void
DRAW_COMMAND_allocate(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "image");
  ASSERT_SLOT_TYPE(vm, 2, LIST, "parameters");

  DRAW_COMMAND* command = (DRAW_COMMAND*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(DRAW_COMMAND));

  IMAGE* image = wrenGetSlotForeign(vm, 1);
  *command = DRAW_COMMAND_init(image);

  wrenGetListElement(vm, 2, 0, 1);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "angle");
  command->angle = wrenGetSlotDouble(vm, 1);

  wrenGetListElement(vm, 2, 1, 1);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "scale.x");
  command->scale.x = wrenGetSlotDouble(vm, 1);

  wrenGetListElement(vm, 2, 2, 1);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "scale.y");
  command->scale.y = wrenGetSlotDouble(vm, 1);

  if (wrenGetListCount(vm, 2) > 3) {
    wrenGetListElement(vm, 2, 3, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "source X");
    command->src.x = wrenGetSlotDouble(vm, 1);

    wrenGetListElement(vm, 2, 4, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "source Y");
    command->src.y = wrenGetSlotDouble(vm, 1);

    wrenGetListElement(vm, 2, 5, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "source width");
    command->srcW = wrenGetSlotDouble(vm, 1);

    wrenGetListElement(vm, 2, 6, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "source height");
    command->srcH = wrenGetSlotDouble(vm, 1);

    wrenGetListElement(vm, 2, 7, 1);
    ASSERT_SLOT_TYPE(vm, 1, STRING, "color mode");
    char* mode = wrenGetSlotString(vm, 1);
    if (STRINGS_EQUAL(mode, "MONO")) {
      command->mode = COLOR_MODE_MONO;
    } else {
      command->mode = COLOR_MODE_RGBA;
    }

    wrenGetListElement(vm, 2, 8, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "foreground color");
    command->foregroundColor = wrenGetSlotDouble(vm, 1);

    wrenGetListElement(vm, 2, 9, 1);
    ASSERT_SLOT_TYPE(vm, 1, NUM, "background color");
    command->backgroundColor = wrenGetSlotDouble(vm, 1);
  }
}

internal void
DRAW_COMMAND_finalize(void* data) {
  // Nothing here
}

internal void
DRAW_COMMAND_draw(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  DRAW_COMMAND* command = wrenGetSlotForeign(vm, 0);

  command->dest.x = wrenGetSlotDouble(vm, 1);
  command->dest.y = wrenGetSlotDouble(vm, 2);

  DRAW_COMMAND_execute(engine, command);
}

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
}

internal void
IMAGE_finalize(void* data) {
  IMAGE* image = data;

  if (image->pixels != NULL) {
    stbi_image_free(image->pixels);
  }
}

internal void
IMAGE_draw(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  int32_t x = wrenGetSlotDouble(vm, 1);
  int32_t y = wrenGetSlotDouble(vm, 2);
  if (image->channels == 2 || image->channels == 4) {
    // drawCommand
    DRAW_COMMAND command = DRAW_COMMAND_init(image);
    command.dest = (VEC){ x, y };
    DRAW_COMMAND_execute(engine, &command);
  } else {
    // fast blit
    size_t height = image->height;
    size_t width = image->width;
    uint32_t* pixels = image->pixels;
    for (size_t j = 0; j < height; j++) {
      uint32_t* row = pixels + (j * width);
      ENGINE_blitLine(engine, x, y + j, width, row);
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

void IMAGE_pget(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 0, FOREIGN, "image");
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");

  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  int64_t width = image->width;
  int64_t height = image->height;
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  if (0 <= x && x < width && 0 <= y && y < height) {
    uint32_t* pixels = image->pixels;
    uint32_t c = pixels[y * width + x];
    wrenSetSlotDouble(vm, 0, c);
  } else {
    VM_ABORT(vm, "pget co-ordinates out of bounds")
  }
}
