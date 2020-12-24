internal void
PLUGIN_load(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* name = wrenGetSlotString(vm, 1);
  PLUGIN_COLLECTION_add(engine, name);
}

