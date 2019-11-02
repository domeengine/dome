typedef struct {
  double x;
  double y;
} VECTOR;

void VECTOR_allocate(WrenVM* vm) {
  VECTOR* point = (VECTOR*)wrenSetSlotNewForeign(vm,
      0, 0, sizeof(VECTOR));
  point->x = wrenGetSlotDouble(vm, 1);
  point->y = wrenGetSlotDouble(vm, 2);
}

void VECTOR_finalize(void* data) {
  // VECTOR* point = data;
}

void VECTOR_getX(WrenVM* vm) {
  VECTOR* point = (VECTOR*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, point->x);
}

void VECTOR_getY(WrenVM* vm) {
  VECTOR* point = (VECTOR*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, point->y);
}

void VECTOR_setX(WrenVM* vm) {
  VECTOR* point = (VECTOR*)wrenGetSlotForeign(vm, 0);
  point->x = wrenGetSlotDouble(vm, 1);
}

void VECTOR_setY(WrenVM* vm) {
  VECTOR* point = (VECTOR*)wrenGetSlotForeign(vm, 0);
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
