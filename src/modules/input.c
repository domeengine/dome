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
  WrenType type = wrenGetSlotType(vm, 1);
  int buttonIndex = 0;
  if (type == WREN_TYPE_STRING) {
    char* buttonName = wrenGetSlotString(vm, 1);
    if (STRINGS_EQUAL(buttonName, "left")) {
      buttonIndex = SDL_BUTTON_LEFT;
    } else if (STRINGS_EQUAL(buttonName, "middle")) {
      buttonIndex = SDL_BUTTON_MIDDLE;
    } else if (STRINGS_EQUAL(buttonName, "right")) {
      buttonIndex = SDL_BUTTON_RIGHT;
    } else {
      VM_ABORT(vm, "Unknown mouse button name");
      return;
    }
  } else if (type == WREN_TYPE_NUM) {
    buttonIndex = wrenGetSlotDouble(vm, 1);
  }

  wrenSetSlotBool(vm, 0, ENGINE_getMouseButton(buttonIndex));
}
