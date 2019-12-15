internal void KEYBOARD_isKeyDown(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* keyName = wrenGetSlotString(vm, 1);
  bool result = ENGINE_getKeyState(engine, keyName);
  wrenSetSlotBool(vm, 0, result);
}

internal void MOUSE_getX(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int x = ENGINE_getMouseX(engine);
  wrenSetSlotDouble(vm, 0, x);
}

internal void MOUSE_getY(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int y = ENGINE_getMouseY(engine);
  wrenSetSlotDouble(vm, 0, y);
}

internal void MOUSE_isButtonPressed(WrenVM* vm) {
  /*
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  WrenType type = wrenGetSlotType(vm, 1);
  if (type == WREN_TYPE_STRING) {

  } else if (type == WREN_TYPE_DOUBLE) {
    int
    ENGINE_getMouseButton()

  }
  */
  wrenSetSlotBool(vm, 0, false);
}
