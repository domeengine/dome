typedef struct {
  const unsigned char* file;
  stbtt_fontinfo info;
} FONT;

typedef struct {
  FONT* font;
  float scale;
  int height;

  bool antialias;
  int32_t offsetY;
} FONT_RASTER;

internal void
FONT_allocate(WrenVM* vm) {
  FONT* font = wrenSetSlotNewForeign(vm, 0, 0, sizeof(FONT));
  int size;
  // ASSERT
  ASSERT_SLOT_TYPE(vm, 1, STRING, "text");
  const unsigned char* file = (const unsigned char*) wrenGetSlotBytes(vm, 1, &size);
  char magic[4] = {0x00, 0x01, 0x00, 0x00};
  if (memcmp(file, magic, 4) != 0) {
    VM_ABORT(vm, "Given file is not a TTF file");
    return;
  }
  font->file = malloc(size * sizeof(char));
  memcpy((void*)font->file, file, size * sizeof(char));
  int result = stbtt_InitFont(&(font->info), font->file, stbtt_GetFontOffsetForIndex(file, 0));

  if (!result) {
    VM_ABORT(vm, "Loading font failed");
  }
}

internal void
FONT_finalize(void* data) {
  FONT* font = data;
  free((void*)font->file);
}

internal void
FONT_RASTER_allocate(WrenVM* vm) {
  FONT_RASTER* raster = wrenSetSlotNewForeign(vm, 0, 0, sizeof(FONT_RASTER));
  ASSERT_SLOT_TYPE(vm, 1, FOREIGN, "font");
  FONT* font = wrenGetSlotForeign(vm, 1);
  raster->font = font;
  raster->antialias = false;
  ASSERT_SLOT_TYPE(vm, 2, NUM, "font size");
  raster->scale = stbtt_ScaleForMappingEmToPixels(&font->info, wrenGetSlotDouble(vm, 2));

  int ascent, descent, linegap;
  stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
  raster->height = (ascent - descent + linegap) * (raster->scale);

  int32_t x0, x1, y0, y1;
  stbtt_GetFontBoundingBox(&font->info, &x0, &x1, &y0, &y1);
  raster->offsetY = (-y0) * raster->scale;
}

internal void
FONT_RASTER_finalize(void* data) {

}

internal void
FONT_RASTER_setAntiAlias(WrenVM* vm) {
  FONT_RASTER* raster = wrenGetSlotForeign(vm, 0);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "antialias");
  raster->antialias = wrenGetSlotBool(vm, 1);
}

internal void
FONT_RASTER_print(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  FONT_RASTER* raster = wrenGetSlotForeign(vm, 0);
  stbtt_fontinfo info = raster->font->info;
  ASSERT_SLOT_TYPE(vm, 1, STRING, "text");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "color");
  const char* text = wrenGetSlotString(vm, 1);
  int64_t x = wrenGetSlotDouble(vm, 2);
  int64_t y = wrenGetSlotDouble(vm, 3);
  uint32_t color = wrenGetSlotDouble(vm, 4);

  unsigned char *bitmap;
  int w, h;

  int fontHeight = raster->height;

  float scale = raster->scale;
  int32_t offsetY = raster->offsetY;

  int32_t posX = x;
  int32_t posY = y;
  int32_t baseY = y - offsetY;
  int len = utf8len(text);
  utf8_int32_t codepoint;
  void* v = utf8codepoint(text, &codepoint);
  for (int charIndex = 0; charIndex < len; charIndex++) {
    if (text[charIndex] == '\n') {
      posX = x;
      baseY += fontHeight;
      v = utf8codepoint(v, &codepoint);
      continue;
    }
    int ax;
    int lsb;
    int oY, oX;
    stbtt_GetCodepointHMetrics(&info, codepoint, &ax, &lsb);
    bitmap = stbtt_GetCodepointBitmap(&info, 0, scale, codepoint, &w, &h, &oX, &oY);
    posX += oX;
    posY = baseY + oY;
    uint32_t outColor;
    float baseAlpha = ((color & 0xFF000000) >> 24) / (float)0xFF;

    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (raster->antialias) {
          uint8_t alpha = baseAlpha * bitmap[j * w + i];
          outColor = (alpha << 24) | (color & 0x00FFFFFF);
        } else {
          outColor = bitmap[j * w + i] > 0 ? color : 0;
        }
        ENGINE_pset(engine, posX + i, posY + j, outColor);
      }
    }
    posX += ax * scale;
    /* add kerning */
    int kern;
    long oldCodepoint = codepoint;
    v = utf8codepoint(v, &codepoint);
    kern = stbtt_GetCodepointKernAdvance(&info, oldCodepoint, codepoint);
    posX += kern * scale;
  }
}
