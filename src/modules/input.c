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
    } else if (STRINGS_EQUAL(buttonName, "X1")) {
      buttonIndex = SDL_BUTTON_X1;
    } else if (STRINGS_EQUAL(buttonName, "X2")) {
      buttonIndex = SDL_BUTTON_X2;
    } else {
      VM_ABORT(vm, "Unknown mouse button name");
      return;
    }
  } else if (type == WREN_TYPE_NUM) {
    buttonIndex = wrenGetSlotDouble(vm, 1);
  }

  wrenSetSlotBool(vm, 0, ENGINE_getMouseButton(buttonIndex));
}

typedef struct {
  int id;
  SDL_GameController* controller;
} GAMEPAD;

internal void
GAMEPAD_allocate(WrenVM* vm) {
  int joystickId = floor(wrenGetSlotDouble(vm, 1));
  if (SDL_IsGameController(joystickId) == SDL_FALSE) {
    VM_ABORT(vm, "Invalid game controller id");
    return;
  }
  SDL_GameController* controller = SDL_GameControllerOpen(joystickId);
  if (controller == NULL) {
    VM_ABORT(vm, "Could not open gamepad");
    return;
  }
  GAMEPAD* gamepad = wrenSetSlotNewForeign(vm, 0, 0, sizeof(GAMEPAD));
  gamepad->controller = controller;
  gamepad->id = joystickId;
  printf("%i", gamepad->id);
}

internal void
GAMEPAD_finalize(void* data) {
  GAMEPAD* gamepad = data;
  SDL_GameControllerClose(gamepad->controller);
}

internal void
GAMEPAD_isButtonPressed(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  int buttonIndex = 0;
  WrenType type = wrenGetSlotType(vm, 1);
  if (type == WREN_TYPE_STRING) {
    char* buttonName = wrenGetSlotString(vm, 1);
    if (STRINGS_EQUAL(buttonName, "up")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_DPAD_UP;
    } else if (STRINGS_EQUAL(buttonName, "left")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    } else if (STRINGS_EQUAL(buttonName, "right")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    } else if (STRINGS_EQUAL(buttonName, "down")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    } else if (STRINGS_EQUAL(buttonName, "start")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_START;
    } else if (STRINGS_EQUAL(buttonName, "back")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_BACK;
    } else if (STRINGS_EQUAL(buttonName, "guide")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_GUIDE;
    } else if (STRINGS_EQUAL(buttonName, "left_shoulder")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    } else if (STRINGS_EQUAL(buttonName, "right_shoulder")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    } else if (STRINGS_EQUAL(buttonName, "A")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_A;
    } else if (STRINGS_EQUAL(buttonName, "B")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_B;
    } else if (STRINGS_EQUAL(buttonName, "X")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_X;
    } else if (STRINGS_EQUAL(buttonName, "Y")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_Y;
    } else {
      VM_ABORT(vm, "Unknown controller button name");
      return;
    }
  } else if (type == WREN_TYPE_NUM) {
    buttonIndex = wrenGetSlotDouble(vm, 1);
  }
  bool isPressed = SDL_GameControllerGetButton(gamepad->controller, buttonIndex);
  wrenSetSlotBool(vm, 0, isPressed);
}

internal void
GAMEPAD_getAnalogStick(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  char* side = wrenGetSlotString(vm, 1);
  int16_t x = 0;
  int16_t y = 0;
  if (STRINGS_EQUAL(side, "left")) {
    x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTX);
    y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTY);
  } else if (STRINGS_EQUAL(side, "right")) {
    x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTX);
    y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTY);
  }

  wrenSetSlotNewList(vm, 0);
  wrenSetSlotDouble(vm, 1, (double)x / SHRT_MAX);
  wrenInsertInList(vm, 0, 0, 1);
  wrenSetSlotDouble(vm, 1, (double)y / SHRT_MAX);
  wrenInsertInList(vm, 0, 1, 1);
}
internal void
GAMEPAD_getTrigger(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  char* side = wrenGetSlotString(vm, 1);
  int triggerIndex = SDL_CONTROLLER_AXIS_INVALID;
  if (STRINGS_EQUAL(side, "left")) {
    triggerIndex = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
  } else if (STRINGS_EQUAL(side, "right")) {
    triggerIndex = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
  }

  int16_t value = SDL_GameControllerGetAxis(gamepad->controller, triggerIndex);
  wrenSetSlotDouble(vm, 0, (double)value / SHRT_MAX);
}
