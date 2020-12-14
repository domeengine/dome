internal void
PLATFORM_getTime(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, (uint32_t)time(NULL));
}

internal void
PLATFORM_getName(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, SDL_GetPlatform());
}
