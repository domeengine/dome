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
  double opacity;

  int32_t srcW;
  int32_t srcH;

  iVEC src;
  VEC dest;

  COLOR_MODE mode;
  // MONO color palette
  uint32_t backgroundColor;
  uint32_t foregroundColor;
  // Tint color value
  uint32_t tintColor;
} DRAW_COMMAND;

internal DRAW_COMMAND
DRAW_COMMAND_init(IMAGE* image) {
  DRAW_COMMAND command;
  command.image = image;

  command.src = (iVEC) { 0, 0 };
  command.srcW = image->width;
  command.srcH = image->height;
  command.dest = (VEC) { 0, 0 };

  command.angle = 0;
  command.scale = (VEC) { 1, 1 };

  command.opacity = 1;

  command.mode = COLOR_MODE_RGBA;
  command.backgroundColor = 0xFF000000;
  command.foregroundColor = 0xFFFFFFFF;
  command.tintColor = 0x00000000;

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
    for (int32_t j = 0; j < h; j++) {
      for (int32_t i = 0; i < w; i++) {

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

        // Prevent out-of-bounds access
        if ((src.x + u) < 0 || (src.x + u) >= image->width || (src.y + v) < 0 || (src.y + v) >= image->height) {
          continue;
        }

        uint32_t preColor = *(pixel + (v * image->width + u));
        uint32_t color = preColor;
        uint8_t alpha = (0xFF000000 & color) >> 24;
        if (command.mode == COLOR_MODE_MONO) {
          if (alpha < 0xFF || (color & 0x00FFFFFF) == 0) {
            color = command.backgroundColor;
          } else {
            color = command.foregroundColor;
          }
        } else {
          color = ((uint8_t)(alpha * command.opacity) << 24) | (preColor & 0x00FFFFFF);
        }
        ENGINE_pset(engine, x, y, color);
        // Only apply the tint on visible pixels
        if (command.tintColor != 0 && color != 0 && (alpha * command.opacity) != 0) {
          ENGINE_pset(engine, x, y, command.tintColor);
        }
      }
    }
  }
}

internal void
DRAW_COMMAND_configure(WrenVM* vm, DRAW_COMMAND* command, int mapSlot, int valueSlot) {
  bool contains;
  wrenEnsureSlots(vm, max(valueSlot, mapSlot) + 1);

  wrenSetSlotString(vm, valueSlot, "angle");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "angle");
    command->angle = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "scaleX");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "scale.x");
    command->scale.x = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "scaleY");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "scale.y");
    command->scale.y = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "srcX");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "source X");
    command->src.x = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "srcY");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "source Y");
    command->src.y = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "srcW");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "source width");
    command->srcW = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "srcH");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "source height");
    command->srcH = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "mode");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, STRING, "color mode");
    const char* mode = wrenGetSlotString(vm, valueSlot);
    if (STRINGS_EQUAL(mode, "MONO")) {
      command->mode = COLOR_MODE_MONO;
    } else {
      command->mode = COLOR_MODE_RGBA;
    }
  }

  wrenSetSlotString(vm, valueSlot, "foreground");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "foreground color");
    command->foregroundColor = wrenGetSlotDouble(vm, valueSlot);
  }
  wrenSetSlotString(vm, valueSlot, "background");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "background color");
    command->backgroundColor = wrenGetSlotDouble(vm, valueSlot);
  }
  wrenSetSlotString(vm, valueSlot, "opacity");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "opacity");
    command->opacity = wrenGetSlotDouble(vm, valueSlot);
  }

  wrenSetSlotString(vm, valueSlot, "tint");
  contains = wrenGetMapContainsKey(vm, mapSlot, valueSlot);
  if (contains) {
    wrenGetMapValue(vm, mapSlot, valueSlot, valueSlot);
    ASSERT_SLOT_TYPE(vm, valueSlot, NUM, "tint color");
    command->tintColor = wrenGetSlotDouble(vm, valueSlot);
  }
}

internal void
DRAW_COMMAND_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 4);
  ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "image");
  ASSERT_SLOT_TYPE(vm, 2, MAP, "parameters");

  DRAW_COMMAND* command = (DRAW_COMMAND*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(DRAW_COMMAND));

  IMAGE* image = wrenGetSlotForeign(vm, 1);
  *command = DRAW_COMMAND_init(image);
  DRAW_COMMAND_configure(vm, command, 2, 3);
}

internal void
DRAW_COMMAND_modify(WrenVM* vm) {
  wrenEnsureSlots(vm, 3);
  ASSERT_SLOT_TYPE(vm, 1, MAP, "parameters");

  DRAW_COMMAND* command = wrenGetSlotForeign(vm, 0);
  DRAW_COMMAND_configure(vm, command, 1, 2);
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

internal void
IMAGE_allocate(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(IMAGE));
  image->pixels = NULL;
  if (wrenGetSlotCount(vm) < 3) {
    image->width = 0;
    image->height = 0;
    image->channels = 0;
  } else {
    ASSERT_SLOT_TYPE(vm, 1, NUM, "width");
    ASSERT_SLOT_TYPE(vm, 2, NUM, "height");
    image->width = wrenGetSlotDouble(vm, 1);
    image->height = wrenGetSlotDouble(vm, 2);
    image->channels = 4;

    image->pixels = calloc(image->width * image->height, image->channels * sizeof(uint8_t));
  }
}

internal void
IMAGE_finalize(void* data) {
  IMAGE* image = data;

  if (image->pixels != NULL) {
    free(image->pixels);
  }
}

internal void
IMAGE_loadFromFile(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 0, FOREIGN, "ImageData");
  ASSERT_SLOT_TYPE(vm, 1, STRING, "image");
  int length;
  const char* fileBuffer = wrenGetSlotBytes(vm, 1, &length);
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  image->pixels = (uint32_t*)stbi_load_from_memory((const stbi_uc*)fileBuffer, length,
      &image->width,
      &image->height,
      &image->channels,
      STBI_rgb_alpha);

  if (image->pixels == NULL)
  {
    const char* errorMsg = stbi_failure_reason();
    size_t errorLength = strlen(errorMsg);
    char buf[errorLength + 8];
    snprintf(buf, errorLength + 8, "Error: %s\n", errorMsg);
    wrenSetSlotString(vm, 0, buf);
    wrenAbortFiber(vm, 0);
    return;
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

internal void
IMAGE_getWidth(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, image->width);
}

internal void
IMAGE_getHeight(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, image->height);
}

internal void
IMAGE_pset(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 0, FOREIGN, "image");
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "color");

  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  int64_t width = image->width;
  int64_t height = image->height;
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  uint64_t color = wrenGetSlotDouble(vm, 3);
  if (0 <= x && x < width && 0 <= y && y < height) {
    uint32_t* pixels = image->pixels;
    pixels[y * width + x] = color;
  } else {
    VM_ABORT(vm, "pset co-ordinates out of bounds")
  }
}

internal void
IMAGE_pget(WrenVM* vm) {
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

internal void
IMAGE_getPNGOutput(void* vm, void* data, int size) {
  wrenSetSlotBytes(vm, 0, data, size);
}

internal void
IMAGE_getPNG(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 0, FOREIGN, "image");

  if (image->pixels != NULL) {
    stbi_write_png_to_func(IMAGE_getPNGOutput, vm, image->width, image->height, image->channels, image->pixels, image->width * sizeof(uint8_t) * image->channels);
  } else {
    wrenSetSlotNull(vm, 0);
  }
}

