
void TILE_allocate(WrenVM* vm) {
  TILE_REF* ref = wren->setSlotNewForeign(vm, 0, 0, sizeof(TILE_REF));
  WrenHandle* handle = wren->getSlotHandle(vm, 1);
  ref->x = wren->getSlotDouble(vm, 2);
  ref->y = wren->getSlotDouble(vm, 3);
  ref->handle = handle;
}

void TILE_finalize(void* data) {
  TILE_REF* ref = data;
  // This is probably a bad idea
  wren->releaseHandle(vm, ref->handle);
  ref->handle = NULL;
}

#define TILE_GETTER(fieldName, method, fieldType) \
  void TILE_get##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  TILE tile; \
  if (renderer->map.width * renderer->map.height > 0) {\
    tile = getTileFrom(ref, renderer); \
  } else { \
   tile = VOID_TILE; \
  } \
  wren->setSlot##fieldType(vm, 0, tile.fieldName); \
}

#define TILE_SETTER(fieldName, method, fieldType) \
void TILE_set##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  if (renderer->map.width * renderer->map.height > 0) {\
    getTileFrom(ref, renderer).fieldName = wren->getSlot##fieldType(vm, 1); \
  } \
}

#define TILE_SETTER_BOOL(fieldName, method) \
void TILE_set##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  if (renderer->map.width * renderer->map.height > 0) {\
    if (wren->getSlotType(vm, 1) == WREN_TYPE_NULL) {\
      getTileFrom(ref, renderer).fieldName = false;\
    } else {\
      getTileFrom(ref, renderer).fieldName = wren->getSlotBool(vm, 1); \
    }\
  } \
}

TILE_GETTER(solid, Solid, Bool)
TILE_SETTER_BOOL(solid, Solid)
TILE_GETTER(door, Door, Bool)
TILE_SETTER_BOOL(door, Door)
TILE_GETTER(state, State, Double)
TILE_SETTER(state, State, Double)
TILE_GETTER(mode, Mode, Double)
TILE_SETTER(mode, Mode, Double)
TILE_GETTER(offset, Offset, Double)
TILE_SETTER(offset, Offset, Double)
TILE_GETTER(thin, Thin, Bool)
TILE_SETTER_BOOL(thin, Thin)
TILE_GETTER(ceiling.id, CeilingTextureId, Double)
TILE_SETTER(ceiling.id, CeilingTextureId, Double)
TILE_GETTER(floor.id, FloorTextureId, Double)
TILE_SETTER(floor.id, FloorTextureId, Double)
TILE_GETTER(wall.id, WallTextureId, Double)
TILE_SETTER(wall.id, WallTextureId, Double)

void TILE_setWallTexture(WrenVM* vm) {
  TILE_REF* ref = wren->getSlotForeign(vm, 0);
  wren->ensureSlots(vm, 3);
  wren->setSlotHandle(vm, 2, ref->handle);
  RENDERER* renderer = wren->getSlotForeign(vm, 2);
  if (renderer->map.width * renderer->map.height > 0) {
    TEXTURE_REF* textureRef = wren->getSlotForeign(vm, 1);
    getTileFrom(ref, renderer).wall = *textureRef;
  }
}


void
TILE_register(DOME_Context ctx) {
  core->registerClass(ctx, "raycaster", "WorldTile", TILE_allocate, TILE_finalize);
  core->registerFn(ctx, "raycaster", "WorldTile.solid", TILE_getSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.solid=(_)", TILE_setSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.door", TILE_getDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.door=(_)", TILE_setDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.state", TILE_getState);
  core->registerFn(ctx, "raycaster", "WorldTile.state=(_)", TILE_setState);
  core->registerFn(ctx, "raycaster", "WorldTile.mode", TILE_getMode);
  core->registerFn(ctx, "raycaster", "WorldTile.mode=(_)", TILE_setMode);
  core->registerFn(ctx, "raycaster", "WorldTile.thin", TILE_getThin);
  core->registerFn(ctx, "raycaster", "WorldTile.thin=(_)", TILE_setThin);
  core->registerFn(ctx, "raycaster", "WorldTile.offset", TILE_getOffset);
  core->registerFn(ctx, "raycaster", "WorldTile.offset=(_)", TILE_setOffset);
  core->registerFn(ctx, "raycaster", "WorldTile.ceilingTextureId", TILE_getCeilingTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.ceilingTextureId=(_)", TILE_setCeilingTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.floorTextureId", TILE_getFloorTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.floorTextureId=(_)", TILE_setFloorTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.wallTextureId", TILE_getWallTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.wallTextureId=(_)", TILE_setWallTextureId);
  core->registerFn(ctx, "raycaster", "WorldTile.wallTexture=(_)", TILE_setWallTexture);

}
