void TEXTURE_REF_allocate(WrenVM* vm) {
  TEXTURE_REF* ref = wren->setSlotNewForeign(vm, 0, 0, sizeof(TEXTURE_REF));
  ref->id = wren->getSlotDouble(vm, 1);
  // TODO: other properties
  ref->min = (V2){0, 0};
  ref->max = (V2){1, 1};
}

void TEXTURE_REF_setMin(WrenVM* vm) {
  TEXTURE_REF* ref = wren->getSlotForeign(vm, 0);
  wren->ensureSlots(vm, 3);
  wren->getListElement(vm, 1, 0, 2);
  ref->min.x = wren->getSlotDouble(vm, 2);
  wren->getListElement(vm, 1, 1, 2);
  ref->min.y = wren->getSlotDouble(vm, 2);
}


void TEXTURE_REF_setMax(WrenVM* vm) {
  TEXTURE_REF* ref = wren->getSlotForeign(vm, 0);
  wren->ensureSlots(vm, 3);
  wren->getListElement(vm, 1, 0, 2);
  ref->max.x = wren->getSlotDouble(vm, 2);
  wren->getListElement(vm, 1, 1, 2);
  ref->max.y = wren->getSlotDouble(vm, 2);
}

#define TEXTURE_REF_SETTER(fieldName, method, fieldType) \
void TEXTURE_REF_set##method(WrenVM* vm) { \
  TEXTURE_REF* ref = wren->getSlotForeign(vm, 0); \
  ref->fieldName = wren->getSlot##fieldType(vm, 1); \
}

TEXTURE_REF_SETTER(id, Id, Double)


void TEXTURE_REF_register(DOME_Context ctx) {
  core->registerClass(ctx, "raycaster", "TextureRef", TEXTURE_REF_allocate, NULL);
  core->registerFn(ctx, "raycaster", "TextureRef.id=(_)", TEXTURE_REF_setId);
  core->registerFn(ctx, "raycaster", "TextureRef.min=(_)", TEXTURE_REF_setMin);
  core->registerFn(ctx, "raycaster", "TextureRef.max=(_)", TEXTURE_REF_setMax);
}
