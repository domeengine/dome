typedef struct {
  stbtt_fontinfo info;
  bool antialias;
} FONT;

typedef struct {
  FONT* font;
  bool antialias;
  float scale;
  int32_t yOffset;
  int32_t pitch;
  char* bitmap;
} FONT_RASTER;

internal void
FONT_allocate(WrenVM* vm) {
  FONT* font = wrenSetSlotNewForeign(vm, 0, 0, sizeof(FONT));
  int size;
  // ASSERT
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
FONT_draw(WrenVM* vm) {
  ENGINE* engine = wrenGetUserData(vm);
  FONT* font = wrenGetSlotForeign(vm, 0);
  char* text = wrenGetSlotString(vm, 1);
  uint64_t color = wrenGetSlotDouble(vm, 2);
  uint64_t size = wrenGetSlotDouble(vm, 3);

  unsigned char *bitmap;
  int w, h;

  float scale = stbtt_ScaleForMappingEmToPixels(&font->info, size);

  int32_t x0, x1, y0, y1;
  stbtt_GetFontBoundingBox(&font->info, &x0, &x1, &y0, &y1);


  int32_t posX = 10;
  int32_t baseY = 50 - (-y0) * scale;
  int32_t posY = 0;
  int len = strlen(text);
  for (int letter = 0; letter < len; letter++) {
    int ax;
    int lsb;
    int oY, oX;
    stbtt_GetCodepointHMetrics(&font->info, text[letter], &ax, &lsb);
    bitmap = stbtt_GetCodepointBitmap(&font->info, 0, scale, text[letter], &w, &h, &oX, &oY);
    posX += oX;
    posY = baseY + oY;
    uint32_t outColor;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (font->antialias) {
          outColor = color | (bitmap[j*w+i] << 24);
        } else {
          outColor = bitmap[j * w + i] > 0 ? color : 0;
        }
        ENGINE_pset(engine, posX + i, posY + j, outColor);
      }
    }
    posX += ax * scale;
    /* add kerning */
    int kern;
    kern = stbtt_GetCodepointKernAdvance(&font->info, text[letter], text[letter + 1]);
    posX += kern * scale;
  }
}

internal void
FONT_finalize(void* data) {
}
