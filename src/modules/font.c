typedef struct {
  stbtt_fontinfo info;
} FONT;

typedef struct {
  FONT* font;
  float scale;
} FONT_RASTER;

internal void FONT_draw(FONT* font);

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

  FONT_draw(font);
}

internal void
FONT_draw(FONT* font) {
  unsigned char *bitmap;
  size_t height = 8;
  int w, h;
  char c = 'a';
  int oX, oY;
  bitmap = stbtt_GetCodepointBitmap(&font->info, 0,stbtt_ScaleForPixelHeight(&font->info, height), c, &w, &h, &oX,&oY);
  for (int j = 0; j < h; j++) {
    for (int i=0; i < w; i++) {
      putchar(bitmap[j*w+i] < 0x0F ? ' ' : '#');
    }
    putchar('\n');
  }
}

internal void
FONT_finalize(void* data) {
}
