typedef struct {
  int32_t width;
  int32_t height;
  int32_t channels;
  uint32_t* pixels;
} IMAGE;

typedef enum { COLOR_MODE_RGBA, COLOR_MODE_MONO } COLOR_MODE;

typedef struct {
  IMAGE* image;
  double scaleX;
  double scaleY;
  double angle;

  int32_t srcX;
  int32_t srcY;
  int32_t srcW;
  int32_t srcH;

  int32_t destX;
  int32_t destY;

  COLOR_MODE mode;
  // MONO colour palette
  uint32_t backgroundColor;
  uint32_t foregroundColor;
} DRAW_COMMAND;

DRAW_COMMAND DRAW_COMMAND_init(IMAGE* image) {
  DRAW_COMMAND command;
  command.image = image;

  command.srcX = 0;
  command.srcY = 0;
  command.srcW = image->width;
  command.srcH = image->height;
  command.destX = 0;
  command.destY = 0;

  command.angle = 0;
  command.scaleX = 1;
  command.scaleY = 1;

  command.mode = COLOR_MODE_RGBA;
  command.backgroundColor = 0xFF000000;
  command.foregroundColor = 0xFFFFFFFF;

  return command;
}

internal void
DRAW_COMMAND_execute(ENGINE* engine, DRAW_COMMAND* commandPtr) {

  DRAW_COMMAND command = *commandPtr;
  IMAGE* image = command.image;

  int32_t srcX = command.srcX;
  int32_t srcY = command.srcY;
  int32_t srcW = command.srcW;
  int32_t srcH = command.srcH;

  int32_t destX = command.destX;
  int32_t destY = command.destY;

  double angle = command.angle;
  double scaleX = command.scaleX;
  double scaleY = command.scaleY;

  double areaHeight = mid(0.0, srcH, image->height);
  double areaWidth = mid(0.0, srcW, image->width);

  double theta = M_PI * (angle / 180.0);
  double c = cos(-theta);
  double s = sin(-theta);

  double sX = (1.0 / scaleX);
  double sY = (1.0 / scaleY);

  double w = (fabs(scaleX) * (srcW) / 2.0) - 0.5;
  double h = (fabs(scaleY) * (srcH) / 2.0) - 0.5;

  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int32_t j = 0; j < ceil(fabs(scaleY)*areaHeight); j++) {
    for (int32_t i = 0; i < ceil(fabs(scaleX)*areaWidth); i++) {
      int32_t x = destX + i;
      int32_t y = destY + j;

      double q = i - w;
      double t = j - h;

      int32_t u = (srcX + ((c * q * sX - s * t * sY) + w * fabs(sX)));
      int32_t v = (srcY + ((s * q * sX + c * t * sY) + h * fabs(sY)));

      // Make sure we are in the selected bounds
      if (u < srcX || u > srcX + srcW || v < srcY || v > srcY + srcH) {
        continue;
      }

      // protect against invalid memory access
      if (v < 0 || v >= image->height || u < 0 || u >= image->width) {
        continue;
      }

      uint32_t color = pixel[v * image->width + u];
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
  ASSERT_SLOT_TYPE(vm, 1, NUM, "scaleX");
  command->scaleX = wrenGetSlotDouble(vm, 1);

  wrenGetListElement(vm, 2, 2, 1);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "scaleY");
  command->scaleY = wrenGetSlotDouble(vm, 1);
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

  command->destX = wrenGetSlotDouble(vm, 1);
  command->destY = wrenGetSlotDouble(vm, 2);

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
  DRAW_COMMAND command = DRAW_COMMAND_init(image);

  command.srcX = wrenGetSlotDouble(vm, 1);
  command.srcY = wrenGetSlotDouble(vm, 2);
  command.srcW = wrenGetSlotDouble(vm, 3);
  command.srcH = wrenGetSlotDouble(vm, 4);
  command.destX = wrenGetSlotDouble(vm, 5);
  command.destY = wrenGetSlotDouble(vm, 6);

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  DRAW_COMMAND_execute(engine, &command);
}

void IMAGE_draw(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  DRAW_COMMAND command = DRAW_COMMAND_init(image);
  command.destX = wrenGetSlotDouble(vm, 1);
  command.destY = wrenGetSlotDouble(vm, 2);
  DRAW_COMMAND_execute(engine, &command);
}
