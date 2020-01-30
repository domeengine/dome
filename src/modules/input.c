internal void KEYBOARD_isKeyDown(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, STRING, "key name");
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

internal void
MOUSE_setHidden(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "hidden");
  bool hidden = wrenGetSlotBool(vm, 1);
  SDL_ShowCursor(hidden ? SDL_DISABLE : SDL_ENABLE);
}

internal void
MOUSE_getHidden(WrenVM* vm) {
  bool shown = SDL_ShowCursor(SDL_QUERY);
  wrenSetSlotBool(vm, 0, !shown);
}

internal void MOUSE_isButtonPressed(WrenVM* vm) {
  WrenType type = wrenGetSlotType(vm, 1);
  int buttonIndex = 0;
  if (type == WREN_TYPE_STRING) {
    char* buttonName = strToLower(wrenGetSlotString(vm, 1));
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
    free(buttonName);
  } else if (type == WREN_TYPE_NUM) {
    buttonIndex = wrenGetSlotDouble(vm, 1);
  } else {
    VM_ABORT(vm, "Invalid button index given")
    return;
  }

  wrenSetSlotBool(vm, 0, ENGINE_getMouseButton(buttonIndex));
}

typedef struct {
  int instanceId;
  SDL_GameController* controller;
} GAMEPAD;

internal void
GAMEPAD_allocate(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "joystick id");
  int joystickId = floor(wrenGetSlotDouble(vm, 1));
  GAMEPAD* gamepad = wrenSetSlotNewForeign(vm, 0, 0, sizeof(GAMEPAD));

  if (joystickId == -1 || SDL_IsGameController(joystickId) == SDL_FALSE) {
    gamepad->controller = NULL;
    gamepad->instanceId = -1;
    return;
  }
  SDL_GameController* controller = SDL_GameControllerOpen(joystickId);
  if (controller == NULL) {
    VM_ABORT(vm, "Could not open gamepad");
    return;
  }
  gamepad->instanceId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
  gamepad->controller = controller;
}

internal void
closeController(GAMEPAD* gamepad) {
  if (gamepad->controller != NULL) {
    SDL_GameControllerClose(gamepad->controller);
    gamepad->controller = NULL;
  }
}

internal void
GAMEPAD_close(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  closeController(gamepad);
}


internal void
GAMEPAD_finalize(void* data) {
  closeController((GAMEPAD*)data);
}

internal void
GAMEPAD_isButtonPressed(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  if (gamepad->controller == NULL) {
    wrenSetSlotBool(vm, 0, false);
    return;
  }
  int buttonIndex = 0;
  WrenType type = wrenGetSlotType(vm, 1);
  if (type == WREN_TYPE_STRING) {
    char* buttonName = strToLower(wrenGetSlotString(vm, 1));
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
    } else if (STRINGS_EQUAL(buttonName, "leftstick")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_LEFTSTICK;
    } else if (STRINGS_EQUAL(buttonName, "rightstick")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    } else if (STRINGS_EQUAL(buttonName, "leftshoulder")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    } else if (STRINGS_EQUAL(buttonName, "rightshoulder")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    } else if (STRINGS_EQUAL(buttonName, "a")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_A;
    } else if (STRINGS_EQUAL(buttonName, "b")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_B;
    } else if (STRINGS_EQUAL(buttonName, "x")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_X;
    } else if (STRINGS_EQUAL(buttonName, "y")) {
      buttonIndex = SDL_CONTROLLER_BUTTON_Y;
    } else {
      VM_ABORT(vm, "Unknown controller button name");
      return;
    }
    free(buttonName);
  } else if (type == WREN_TYPE_NUM) {
    buttonIndex = wrenGetSlotDouble(vm, 1);
  } else {
    VM_ABORT(vm, "Invalid controller button index")
    return;
  }
  bool isPressed = SDL_GameControllerGetButton(gamepad->controller, buttonIndex);
  wrenSetSlotBool(vm, 0, isPressed);
}

internal void
GAMEPAD_getAnalogStick(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  int16_t x = 0;
  int16_t y = 0;
  if (gamepad->controller != NULL) {
    ASSERT_SLOT_TYPE(vm, 1, STRING, "analog stick side");
    char* side = strToLower(wrenGetSlotString(vm, 1));
    if (STRINGS_EQUAL(side, "left")) {
      x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTX);
      y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTY);
    } else if (STRINGS_EQUAL(side, "right")) {
      x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTX);
      y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTY);
    }
    free(side);
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
  if (gamepad->controller == NULL) {
    wrenSetSlotDouble(vm, 0, 0.0);
    return;
  }
  ASSERT_SLOT_TYPE(vm, 1, STRING, "trigger side");
  char* side = strToLower(wrenGetSlotString(vm, 1));
  int triggerIndex = SDL_CONTROLLER_AXIS_INVALID;
  if (STRINGS_EQUAL(side, "left")) {
    triggerIndex = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
  } else if (STRINGS_EQUAL(side, "right")) {
    triggerIndex = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
  }
  free(side);

  int16_t value = SDL_GameControllerGetAxis(gamepad->controller, triggerIndex);
  wrenSetSlotDouble(vm, 0, (double)value / SHRT_MAX);
}

internal void
GAMEPAD_isAttached(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  if (gamepad->controller == NULL) {
    wrenSetSlotBool(vm, 0, false);
    return;
  }
  wrenSetSlotBool(vm, 0, SDL_GameControllerGetAttached(gamepad->controller) == SDL_TRUE);
}

internal void
GAMEPAD_getName(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  if (gamepad->controller == NULL) {
    wrenSetSlotString(vm, 0, "NONE");
    return;
  }
  wrenSetSlotString(vm, 0, SDL_GameControllerName(gamepad->controller));
}

internal void
GAMEPAD_getId(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  if (gamepad->controller == NULL) {
    wrenSetSlotDouble(vm, 0, -1);
    return;
  }
  wrenSetSlotDouble(vm, 0, gamepad->instanceId);
}

internal void
GAMEPAD_getGamePadIds(WrenVM* vm) {
  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
  int maxJoysticks = SDL_NumJoysticks();
  int listCount = 0;
  wrenEnsureSlots(vm, 2);
  wrenSetSlotNewList(vm, 0);
  for(int joystickId = 0; joystickId < maxJoysticks; joystickId++) {
    if (!SDL_IsGameController(joystickId)) {
      continue;
    }
    wrenSetSlotDouble(vm, 1, joystickId);
    wrenInsertInList(vm, 0, listCount, 1);
    listCount++;
  }
}

internal void
GAMEPAD_eventAdded(WrenVM* vm, int joystickId) {
  WrenHandle* addedMethod = wrenMakeCallHandle(vm, "addGamePad(_)");
  wrenSetSlotDouble(vm, 1, joystickId);
  wrenGetVariable(vm, "input", "GamePad", 0);
  wrenCall(vm, addedMethod);
  wrenReleaseHandle(vm, addedMethod);
}


internal void
GAMEPAD_eventRemoved(WrenVM* vm, int instanceId) {
  WrenHandle* removeMethod = wrenMakeCallHandle(vm, "removeGamePad(_)");
  wrenSetSlotDouble(vm, 1, instanceId);
  wrenGetVariable(vm, "input", "GamePad", 0);
  wrenCall(vm, removeMethod);
  wrenReleaseHandle(vm, removeMethod);
}
