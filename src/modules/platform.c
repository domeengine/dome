internal void
PLATFORM_getTime(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, (uint32_t)time(NULL));
}
