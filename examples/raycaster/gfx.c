void vLine(DOME_Context ctx, int32_t x, int32_t y0, uint32_t y1, DOME_Color color) {
  uint32_t height = canvas->getHeight(ctx);
  y0 = fmax(0, y0);
  y1 = fmin(y1, height - 1);
  for (int y = y0; y <= y1; y++) {
    unsafePset(ctx, x, y, color);
  }
}
