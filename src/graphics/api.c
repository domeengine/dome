internal void
GRAPHICS_API_unsafePset(DOME_Context ctx, int32_t x, int32_t y, DOME_Color color) {
  ENGINE* engine = (ENGINE*)ctx;
  ENGINE_unsafePset(engine, x, y, color.value);
}

internal void
GRAPHICS_API_pset(DOME_Context ctx, int32_t x, int32_t y, DOME_Color color) {
  ENGINE* engine = (ENGINE*)ctx;
  ENGINE_pset(engine, x, y, color.value);
}

internal DOME_Color
GRAPHICS_API_pget(DOME_Context ctx, int32_t x, int32_t y) {
  ENGINE* engine = (ENGINE*)ctx;
  DOME_Color color;
  color.value = ENGINE_pget(engine, x, y);
  printf("%02X\n", color.value);
  return color;
}

internal uint32_t
GRAPHICS_API_getWidth(DOME_Context ctx) {
  ENGINE* engine = (ENGINE*)ctx;
  return engine->canvas.width;
}
internal uint32_t
GRAPHICS_API_getHeight(DOME_Context ctx) {
  ENGINE* engine = (ENGINE*)ctx;
  return engine->canvas.height;
}

GRAPHICS_API_v0 graphics_v0 = {
  .pget = GRAPHICS_API_pget,
  .pset = GRAPHICS_API_pset,
  .unsafePset = GRAPHICS_API_unsafePset,
  .getWidth = GRAPHICS_API_getWidth,
  .getHeight = GRAPHICS_API_getHeight,
};
