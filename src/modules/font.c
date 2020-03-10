typedef struct {
  stbtt_fontinfo info;
} FONT;

typedef struct {
  FONT* font;
  float scale;

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
  int result = stbtt_InitFont(&(font->info), file, stbtt_GetFontOffsetForIndex(file, 0));

  if (!result) {
    VM_ABORT(vm, "Loading font failed");
  }
}

internal void
FONT_finalize(void* data) {
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
  char* text = wrenGetSlotString(vm, 1);
  int64_t x = wrenGetSlotDouble(vm, 2);
  int64_t y = wrenGetSlotDouble(vm, 3);
  uint32_t color = wrenGetSlotDouble(vm, 4);

  unsigned char *bitmap;
  int w, h;

  float scale = raster->scale;
  int32_t offsetY = raster->offsetY;

  int32_t posX = x;
  int32_t posY = y;
  int32_t baseY = y - offsetY;
  int len = strlen(text);
  for (int letter = 0; letter < len; letter++) {
    int ax;
    int lsb;
    int oY, oX;
    stbtt_GetCodepointHMetrics(&info, text[letter], &ax, &lsb);
    bitmap = stbtt_GetCodepointBitmap(&info, 0, scale, text[letter], &w, &h, &oX, &oY);
    posX += oX;
    posY = baseY + oY;
    uint32_t outColor;

    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (raster->antialias) {
          outColor = (bitmap[j * w + i] << 24) | (color & 0x00FFFFFF);
        } else {
          outColor = bitmap[j * w + i] > 0 ? color : 0;
        }
        ENGINE_pset(engine, posX + i, posY + j, outColor);
      }
    }
    posX += ax * scale;
    /* add kerning */
    int kern;
    kern = stbtt_GetCodepointKernAdvance(&info, text[letter], text[letter + 1]);
    posX += kern * scale;
  }
}
