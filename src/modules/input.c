internal void KEYBOARD_isKeyDown(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* keyName = wrenGetSlotString(vm, 1);
  bool result = ENGINE_getKeyState(engine, keyName);
  wrenSetSlotBool(vm, 0, result);
}
