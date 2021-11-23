
void OBJ_allocate(WrenVM* vm) {
  OBJ_REF* ref = wren->setSlotNewForeign(vm, 0, 0, sizeof(OBJ_REF));
  WrenHandle* handle = wren->getSlotHandle(vm, 1);
  ref->handle = handle;
  ref->id = wren->getSlotDouble(vm, 2);
}

void OBJ_finalize(void* data) {
  OBJ_REF* ref = data;
  // This is probably a bad idea
  wren->releaseHandle(vm, ref->handle);
  ref->id = 0;
  ref->handle = NULL;
}

void OBJ_remove(WrenVM* vm) {
  OBJ_REF* ref = wren->getSlotForeign(vm, 0);
  wren->ensureSlots(vm, 2);
  wren->setSlotHandle(vm, 1, ref->handle);
  RENDERER* renderer = wren->getSlotForeign(vm, 1);

  uint64_t id = ref->id;
  size_t count = renderer->objectCount;

  for (size_t i = 0; i < count; i++) {
    OBJ* item = &(renderer->objects[i]);
    if (item->id == id) {
      uint64_t last = count - 1;
      renderer->objects[id] = renderer->objects[last];
      renderer->objects[last].id = 0;
      renderer->objectCount--;
      break;
    }
  }
  // Handle if ID not found?
}

#define OBJ_GETTER(fieldName, method, fieldType) \
  void OBJ_get##method(WrenVM* vm) { \
  OBJ_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  OBJ* obj = RENDERER_getObject(renderer, ref->id); \
  wren->setSlot##fieldType(vm, 0, obj->fieldName); \
}

#define OBJ_SETTER(fieldName, method, fieldType) \
void OBJ_set##method(WrenVM* vm) { \
  OBJ_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  OBJ* obj = RENDERER_getObject(renderer, ref->id); \
  obj->fieldName = wren->getSlot##fieldType(vm, 1); \
}

OBJ_GETTER(id, Id, Double)
OBJ_GETTER(textureId, TextureId, Double)
OBJ_SETTER(textureId, TextureId, Double)
OBJ_GETTER(position.x, X, Double)
OBJ_SETTER(position.x, X, Double)
OBJ_GETTER(position.y, Y, Double)
OBJ_SETTER(position.y, Y, Double)
OBJ_GETTER(div.x, UDiv, Double)
OBJ_SETTER(div.x, UDiv, Double)
OBJ_GETTER(div.y, VDiv, Double)
OBJ_SETTER(div.y, VDiv, Double)
OBJ_GETTER(vMove, VMove, Double)
OBJ_SETTER(vMove, VMove, Double)

void OBJ_register(DOME_Context ctx) {
  core->registerClass(ctx, "raycaster", "WorldObject", OBJ_allocate, OBJ_finalize);
  core->registerFn(ctx, "raycaster", "WorldObject.remove()", OBJ_remove);
  core->registerFn(ctx, "raycaster", "WorldObject.id", OBJ_getId);
  core->registerFn(ctx, "raycaster", "WorldObject.textureId", OBJ_getTextureId);
  core->registerFn(ctx, "raycaster", "WorldObject.textureId=(_)", OBJ_setTextureId);
  core->registerFn(ctx, "raycaster", "WorldObject.x", OBJ_getX);
  core->registerFn(ctx, "raycaster", "WorldObject.x=(_)", OBJ_setX);
  core->registerFn(ctx, "raycaster", "WorldObject.y", OBJ_getY);
  core->registerFn(ctx, "raycaster", "WorldObject.y=(_)", OBJ_setY);
  core->registerFn(ctx, "raycaster", "WorldObject.uDiv", OBJ_getUDiv);
  core->registerFn(ctx, "raycaster", "WorldObject.uDiv=(_)", OBJ_setUDiv);
  core->registerFn(ctx, "raycaster", "WorldObject.vDiv", OBJ_getVDiv);
  core->registerFn(ctx, "raycaster", "WorldObject.vDiv=(_)", OBJ_setVDiv);
  core->registerFn(ctx, "raycaster", "WorldObject.vMove", OBJ_getVMove);
  core->registerFn(ctx, "raycaster", "WorldObject.vMove=(_)", OBJ_setVMove);


}
