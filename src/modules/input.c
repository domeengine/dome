// Inspired by the SDL2 approach to button mapping

global_variable const char* controllerButtonMap[] = {
  "a",
  "b",
  "x",
  "y",
  "back",
  "guide",
  "start",
  "leftstick",
  "rightstick",
  "leftshoulder",
  "rightshoulder",
  "up",
  "down",
  "left",
  "right",
  NULL
};
global_variable bool inputCaptured = false;
global_variable WrenHandle* commitMethod = NULL;
global_variable WrenHandle* gamePadAddMethod = NULL;
global_variable WrenHandle* gamePadLookupMethod = NULL;
global_variable WrenHandle* gamePadRemoveMethod = NULL;
global_variable WrenHandle* keyboardClearTextMethod = NULL;
global_variable WrenHandle* keyboardAddTextMethod = NULL;
global_variable WrenHandle* keyboardSetCompositionTextMethod = NULL;

internal void
INPUT_capture(WrenVM* vm) {
  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

  if (!inputCaptured) {
    wrenGetVariable(vm, "input", "Keyboard", 0);
    keyboardClearTextMethod = wrenMakeCallHandle(vm, "clearText()");
    keyboardAddTextMethod = wrenMakeCallHandle(vm, "addText(_)");
    keyboardSetCompositionTextMethod = wrenMakeCallHandle(vm, "setComposition(_,_,_)");
    keyboardClass = wrenGetSlotHandle(vm, 0);

    wrenGetVariable(vm, "input", "Mouse", 0);
    mouseClass = wrenGetSlotHandle(vm, 0);

    wrenGetVariable(vm, "input", "GamePad", 0);
    gamepadClass  = wrenGetSlotHandle(vm, 0);

    updateInputMethod = wrenMakeCallHandle(vm, "update(_,_)");
    commitMethod = wrenMakeCallHandle(vm, "commit()");
    gamePadAddMethod = wrenMakeCallHandle(vm, "addGamePad(_)");
    gamePadLookupMethod = wrenMakeCallHandle(vm, "[_]");
    gamePadRemoveMethod = wrenMakeCallHandle(vm, "removeGamePad(_)");
    inputCaptured = true;
  }
}

internal WrenInterpretResult
INPUT_commit(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, keyboardClass);
  WrenInterpretResult interpreterResult = wrenCall(vm, commitMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    return interpreterResult;
  }

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  if (engine->mouse.relative) {
    SDL_GetRelativeMouseState(&(engine->mouse.x), &(engine->mouse.y));
  } else {
    SDL_GetMouseState(&(engine->mouse.x), &(engine->mouse.y));
  }

  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, mouseClass);
  interpreterResult = wrenCall(vm, commitMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    return interpreterResult;
  }
  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, gamepadClass);
  interpreterResult = wrenCall(vm, commitMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    return interpreterResult;
  }

  return WREN_RESULT_SUCCESS;
}



internal void
INPUT_release(WrenVM* vm) {
  if (inputCaptured) {
    wrenReleaseHandle(vm, keyboardClass);
    wrenReleaseHandle(vm, mouseClass);
    wrenReleaseHandle(vm, gamepadClass);
    wrenReleaseHandle(vm, updateInputMethod);
    wrenReleaseHandle(vm, commitMethod);
    wrenReleaseHandle(vm, gamePadAddMethod);
    wrenReleaseHandle(vm, gamePadLookupMethod);
    wrenReleaseHandle(vm, gamePadRemoveMethod);
    wrenReleaseHandle(vm, keyboardClearTextMethod);
    wrenReleaseHandle(vm, keyboardAddTextMethod);
    wrenReleaseHandle(vm, keyboardSetCompositionTextMethod);
    inputCaptured = false;
  }
}

typedef enum {
  DOME_INPUT_KEYBOARD,
  DOME_INPUT_MOUSE,
  DOME_INPUT_CONTROLLER
} DOME_INPUT_TYPE;

internal WrenInterpretResult
INPUT_clearText(WrenVM* vm) {
  if (!inputCaptured) {
    return WREN_RESULT_SUCCESS;
  }
  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, keyboardClass);
  return wrenCall(vm, keyboardClearTextMethod);
}

internal WrenInterpretResult
INPUT_addText(WrenVM* vm, char* text) {
  if (!inputCaptured) {
    return WREN_RESULT_SUCCESS;
  }
  wrenEnsureSlots(vm, 2);
  wrenSetSlotHandle(vm, 0, keyboardClass);
  wrenSetSlotString(vm, 1, text);
  return wrenCall(vm, keyboardAddTextMethod);
}

internal WrenInterpretResult
INPUT_setCompositionText(WrenVM* vm, char* text, int start, int length) {
  if (!inputCaptured) {
    return WREN_RESULT_SUCCESS;
  }
  wrenEnsureSlots(vm, 4);
  wrenSetSlotHandle(vm, 0, keyboardClass);
  wrenSetSlotString(vm, 1, text);
  wrenSetSlotDouble(vm, 2, start);
  wrenSetSlotDouble(vm, 3, length);
  return wrenCall(vm, keyboardSetCompositionTextMethod);
}

internal WrenInterpretResult
INPUT_update(WrenVM* vm, DOME_INPUT_TYPE type, const char* inputName, bool state) {
  if (inputCaptured) {
    wrenEnsureSlots(vm, 3);
    switch (type) {
      default:
      case DOME_INPUT_KEYBOARD: wrenSetSlotHandle(vm, 0, keyboardClass); break;
      case DOME_INPUT_MOUSE: wrenSetSlotHandle(vm, 0, mouseClass); break;
      // It's assumed the controller object is preloaded.
      case DOME_INPUT_CONTROLLER: break;
    }
    wrenSetSlotString(vm, 1, inputName);
    wrenSetSlotBool(vm, 2, state);
    return wrenCall(vm, updateInputMethod);
  }
  return WREN_RESULT_SUCCESS;
}

internal void
KEYBOARD_setHandleText(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "handleText");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  bool handleText = wrenGetSlotBool(vm, 1);
  engine->handleText = handleText;
  if (handleText) {
    SDL_StartTextInput();
  } else {
    SDL_StopTextInput();
  }
}


internal void
KEYBOARD_getHandleText(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotBool(vm, 0, engine->handleText);
}

internal void
KEYBOARD_setTextRegion(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  int32_t x = wrenGetSlotDouble(vm, 1);
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  int32_t y = wrenGetSlotDouble(vm, 2);
  ASSERT_SLOT_TYPE(vm, 3, NUM, "w");
  int32_t w = wrenGetSlotDouble(vm, 3);
  ASSERT_SLOT_TYPE(vm, 4, NUM, "h");
  int32_t h = wrenGetSlotDouble(vm, 4);

  DOME_RECT region = {
    .x = x,
    .y = y,
    .w = w,
    .h = h
  };
  engine->textRegion = region;
  ENGINE_updateTextRegion(engine);
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

internal void MOUSE_getScrollX(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotDouble(vm, 0, engine->mouse.scrollX);
}

internal void MOUSE_getScrollY(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotDouble(vm, 0, engine->mouse.scrollY);
}

internal void
MOUSE_setRelative(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "relative");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  bool relative = wrenGetSlotBool(vm, 1);
  ENGINE_setMouseRelative(engine, relative);
}

internal void
MOUSE_getRelative(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotBool(vm, 0, engine->mouse.relative);
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

internal void
MOUSE_setCursor(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  /*
  SDL_FreeCursor(engine->mouse.cursor);
  engine->mouse.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  SDL_SetCursor(engine->mouse.cursor);
  */
  ASSERT_SLOT_TYPE(vm, 1, STRING, "cursorName");
  char* name = (char*)wrenGetSlotString(vm, 1);
  //const char* cursorName = strToLower(wrenGetSlotString(vm, 1));
  ENGINE_setMouseCursor(engine, name);
}

typedef struct {
  int instanceId;
  SDL_GameController* controller;
  SDL_Haptic* haptics;
} GAMEPAD;

internal void
GAMEPAD_allocate(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "joystick id");
  int joystickId = floor(wrenGetSlotDouble(vm, 1));
  GAMEPAD* gamepad = wrenSetSlotNewForeign(vm, 0, 0, sizeof(GAMEPAD));

  if (joystickId == -1 || SDL_IsGameController(joystickId) == SDL_FALSE) {
    gamepad->controller = NULL;
    gamepad->haptics = NULL;
    gamepad->instanceId = -1;
    return;
  }
  SDL_GameController* controller = SDL_GameControllerOpen(joystickId);
  if (controller == NULL) {
    VM_ABORT(vm, "Could not open gamepad");
    return;
  }
  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
  gamepad->instanceId = SDL_JoystickInstanceID(joystick);
  gamepad->controller = controller;
  gamepad->haptics = SDL_HapticOpenFromJoystick(joystick);
  if (SDL_HapticRumbleSupported(gamepad->haptics) == SDL_FALSE
      || SDL_HapticRumbleInit(gamepad->haptics) != 0) {
    SDL_HapticClose(gamepad->haptics);
    gamepad->haptics = NULL;
  }
}

internal void
closeController(GAMEPAD* gamepad) {
  if (gamepad->haptics != NULL) {
    SDL_HapticClose(gamepad->haptics);
  }

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
GAMEPAD_rumble(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 0, FOREIGN, "GamePad");
  ASSERT_SLOT_TYPE(vm, 1, NUM, "strength");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "length");
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  float strength = fmid(0, wrenGetSlotDouble(vm, 1), 1);
  double length = fmax(0, wrenGetSlotDouble(vm, 2));
  SDL_HapticRumblePlay(gamepad->haptics, strength, length);
}

internal void
GAMEPAD_getAnalogStick(WrenVM* vm) {
  GAMEPAD* gamepad = wrenGetSlotForeign(vm, 0);
  int16_t x = 0;
  int16_t y = 0;
  if (gamepad->controller != NULL) {
    ASSERT_SLOT_TYPE(vm, 1, STRING, "analog stick side");
    const char* side = strToLower(wrenGetSlotString(vm, 1));
    if (STRINGS_EQUAL(side, "left")) {
      x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTX);
      y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_LEFTY);
    } else if (STRINGS_EQUAL(side, "right")) {
      x = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTX);
      y = SDL_GameControllerGetAxis(gamepad->controller, SDL_CONTROLLER_AXIS_RIGHTY);
    }
    free((void*)side);
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
  if (inputCaptured) {
    wrenEnsureSlots(vm, 2);
    wrenSetSlotDouble(vm, 1, joystickId);
    wrenSetSlotHandle(vm, 0, gamepadClass);
    wrenCall(vm, gamePadAddMethod);
  }
}

internal WrenInterpretResult
GAMEPAD_eventButtonPressed(WrenVM* vm, int joystickId, const char* buttonName, bool state) {
  if (inputCaptured) {
    wrenEnsureSlots(vm, 3);
    wrenSetSlotDouble(vm, 1, joystickId);
    wrenSetSlotHandle(vm, 0, gamepadClass);
    WrenInterpretResult result = wrenCall(vm, gamePadLookupMethod);
    if (result != WREN_RESULT_SUCCESS) {
      return result;
    }
    // A gamepad instance should be in the 0 slot now
    result = INPUT_update(vm, DOME_INPUT_CONTROLLER, buttonName, state);
    if (result != WREN_RESULT_SUCCESS) {
      return result;
    }
  }

  return WREN_RESULT_SUCCESS;
}

internal void
GAMEPAD_eventRemoved(WrenVM* vm, int instanceId) {
  if (inputCaptured) {
    wrenEnsureSlots(vm, 2);
    wrenSetSlotDouble(vm, 1, instanceId);
    wrenSetSlotHandle(vm, 0, gamepadClass);
    wrenCall(vm, gamePadRemoveMethod);
  }
}

internal const char*
GAMEPAD_stringFromButton(SDL_GameControllerButton button) {
  if (button > SDL_CONTROLLER_BUTTON_INVALID && button < SDL_CONTROLLER_BUTTON_MAX) {
    return controllerButtonMap[button];
  }
  return NULL;
}

internal void
CLIPBOARD_getContent(WrenVM* vm) {
  char* text = SDL_GetClipboardText();
  if (text == NULL) {
    VM_ABORT(vm, SDL_GetError());
    return;
  }
  wrenSetSlotString(vm, 0, text);
  SDL_free(text);
}

internal void
CLIPBOARD_setContent(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "text");
  const char* text = wrenGetSlotString(vm, 1);
  int result = SDL_SetClipboardText(text);
  if (result < 0) {
    VM_ABORT(vm, SDL_GetError());
    return;
  }
}
