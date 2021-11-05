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

GRAPHICS_API_v0 graphics_v0 = {
  // .getWidth = GRAPHICS_API_getWidth,
  // .getHeight = GRAPHICS_API_getHeight,
  .pget = GRAPHICS_API_pget,
  .pset = GRAPHICS_API_pset,
};
