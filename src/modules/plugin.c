internal void
PLUGIN_load(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* name = wrenGetSlotString(vm, 1);
  DOME_Result result = PLUGIN_COLLECTION_add(engine, name);
  if (result != DOME_RESULT_SUCCESS) {
    char buf[256];
    sprintf(buf, "There was a problem initialising plugin: %s", name);
    VM_ABORT(vm, buf);
  }
}

