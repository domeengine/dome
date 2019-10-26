internal void CANVAS_print(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  char* text = (char*)wrenGetSlotString(vm, 1);
  int16_t x = floor(wrenGetSlotDouble(vm, 2));
  int16_t y = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));

  ENGINE_print(engine, text, x, y, c);
}

internal void CANVAS_pset(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  uint32_t c = floor(wrenGetSlotDouble(vm, 3));
  ENGINE_pset(engine, x,y,c);
}

internal void CANVAS_circle_filled(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t r = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));
  ENGINE_circle_filled(engine, x, y, r, c);
}

internal void CANVAS_circle(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t r = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));
  ENGINE_circle(engine, x, y, r, c);
}
internal void CANVAS_line(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x1 = floor(wrenGetSlotDouble(vm, 1));
  int16_t y1 = floor(wrenGetSlotDouble(vm, 2));
  int16_t x2 = floor(wrenGetSlotDouble(vm, 3));
  int16_t y2 = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_line(engine, x1, y1, x2, y2, c);
}

internal void CANVAS_ellipse(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x1 = floor(wrenGetSlotDouble(vm, 1));
  int16_t y1 = floor(wrenGetSlotDouble(vm, 2));
  int16_t x2 = floor(wrenGetSlotDouble(vm, 3));
  int16_t y2 = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_ellipse(engine, x1, y1, x2, y2, c);
}

internal void CANVAS_ellipsefill(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x1 = floor(wrenGetSlotDouble(vm, 1));
  int16_t y1 = floor(wrenGetSlotDouble(vm, 2));
  int16_t x2 = floor(wrenGetSlotDouble(vm, 3));
  int16_t y2 = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_ellipsefill(engine, x1, y1, x2, y2, c);
}

internal void CANVAS_rect(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t w = floor(wrenGetSlotDouble(vm, 3));
  int16_t h = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_rect(engine, x, y, w, h, c);
}
internal void CANVAS_rectfill(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t w = floor(wrenGetSlotDouble(vm, 3));
  int16_t h = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_rectfill(engine, x, y, w, h, c);
}
