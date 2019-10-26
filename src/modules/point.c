typedef struct {
  double x;
  double y;
} POINT;

void POINT_allocate(WrenVM* vm) {
  POINT* point = (POINT*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(POINT));
  point->x = wrenGetSlotDouble(vm, 1);
  point->y = wrenGetSlotDouble(vm, 2);
}

void POINT_finalize(void* data) {
  // POINT* point = data;
}

void POINT_getX(WrenVM* vm) {
  POINT* point = (POINT*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, point->x);
}

void POINT_getY(WrenVM* vm) {
  POINT* point = (POINT*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, point->y);
}

void POINT_setX(WrenVM* vm) {
  POINT* point = (POINT*)wrenGetSlotForeign(vm, 0);
  point->x = wrenGetSlotDouble(vm, 1);
}

void POINT_setY(WrenVM* vm) {
  POINT* point = (POINT*)wrenGetSlotForeign(vm, 0);
  point->y = wrenGetSlotDouble(vm, 1);
}

/*
void IMAGE_draw(WrenVM* vm) {
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  IMAGE* image = (IMAGE*)wrenGetSlotForeign(vm, 0);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);


  uint32_t* pixel = (uint32_t*)image->pixels;
  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      uint32_t c = pixel[j * image->width + i];
      ENGINE_pset(engine, x+i, y+j, c);
    }
  }
}
*/
