// This informs the engine we want to stop running, and jumps to the end of the game loop if we have no errors to report.
internal void
PROCESS_exit(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "code");
  engine->running = false;
  engine->exit_status = floor(wrenGetSlotDouble(vm, 1));
  wrenSetSlotNull(vm, 0);
}


internal void
WINDOW_resize(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "width");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "height");
  uint32_t width = wrenGetSlotDouble(vm, 1);
  uint32_t height = wrenGetSlotDouble(vm, 2);
  SDL_SetWindowSize(engine->window, width, height);
}

internal void
WINDOW_getWidth(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int width = 0;
  SDL_GetWindowSize(engine->window, &width, NULL);
  wrenSetSlotDouble(vm, 0, width);
}

internal void
WINDOW_getHeight(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int height = 0;
  SDL_GetWindowSize(engine->window, NULL, &height);
  wrenSetSlotDouble(vm, 0, height);
}

internal void
WINDOW_setTitle(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, STRING, "title");
  char* title = wrenGetSlotString(vm, 1);
  SDL_SetWindowTitle(engine->window, title);
}

internal void
WINDOW_getTitle(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotString(vm, 0, SDL_GetWindowTitle(engine->window));
}

internal void
WINDOW_setVsync(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "vsync");
  bool value = wrenGetSlotBool(vm, 1);
  ENGINE_setupRenderer(engine, value);
}

internal void
WINDOW_setLockStep(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  engine->lockstep = wrenGetSlotBool(vm, 1);
}

internal void
WINDOW_setFullscreen(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "fullscreen");
  bool value = wrenGetSlotBool(vm, 1);
  SDL_SetWindowFullscreen(engine->window, value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

internal void
WINDOW_getFullscreen(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  uint32_t flags = SDL_GetWindowFlags(engine->window);
  wrenSetSlotBool(vm, 0, (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0);
}

internal void
VERSION_getString(WrenVM* vm) {
  size_t len = 0;
  char* version = DOME_VERSION;
  if (version[len] == 'v') {
    version++;
  }
  for (len = 0; len < strlen(version); len++) {
    if (version[len] != '.' && !isdigit(version[len])) {
      break;
    }
  }
  printf("len: %lu \n", len);
  wrenSetSlotBytes(vm, 0, version, len);
}

// JSON parser
const char json_typename[][32] = {
    [JSON_ERROR]      = "com.domeengine:JSON_ERROR",
    [JSON_DONE]       = "com.domeengine:JSON_DONE",
    [JSON_OBJECT]     = "com.domeengine:JSON_OBJECT",
    [JSON_OBJECT_END] = "com.domeengine:JSON_OBJECT_END",
    [JSON_ARRAY]      = "com.domeengine:JSON_ARRAY",
    [JSON_ARRAY_END]  = "com.domeengine:JSON_ARRAY_END",
    [JSON_STRING]     = "com.domeengine:JSON_STRING",
    [JSON_NUMBER]     = "com.domeengine:JSON_NUMBER",
    [JSON_TRUE]       = "com.domeengine:JSON_TRUE",
    [JSON_FALSE]      = "com.domeengine:JSON_FALSE",
    [JSON_NULL]       = "com.domeengine:JSON_NULL",
};

json_stream jsonStream[1];

internal void
JSON_streamBegin(WrenVM * vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "value");
  char * value = wrenGetSlotString(vm, 1);
  json_open_string(jsonStream, value);
  json_set_streaming(jsonStream, 1);
}

internal void
JSON_streamEnd(WrenVM * vm) {
  json_reset(jsonStream);
  json_close(jsonStream);
}

internal void
JSON_value(WrenVM * vm) {
  const char * value = json_get_string(jsonStream, 0);
  wrenSetSlotString(vm, 0, value);
}

internal void
JSON_error_message(WrenVM * vm) {
  const char * error = json_get_error(jsonStream);
  if(error) {
    wrenSetSlotString(vm, 0, error);
    return;
  }
  wrenSetSlotString(vm, 0, "");
}

internal void
JSON_lineno(WrenVM * vm) {
  wrenSetSlotDouble(vm, 0, json_get_lineno(jsonStream));
}

internal void
JSON_pos(WrenVM * vm) {
  wrenSetSlotDouble(vm, 0, json_get_position(jsonStream));
}

internal void
JSON_next(WrenVM * vm) {
  enum json_type type = json_next(jsonStream);
  switch (type) {
      case JSON_NULL:
      case JSON_TRUE:
      case JSON_FALSE:
      case JSON_NUMBER:
      case JSON_STRING:
      case JSON_ARRAY:
      case JSON_OBJECT:
      case JSON_OBJECT_END:
      case JSON_ARRAY_END:
      case JSON_DONE:
      case JSON_ERROR:
          wrenSetSlotString(vm, 0, json_typename[type]);
          break;
  }
}

// We have to use C functions for escaping chars
// because a bug in compiler throws error when using \ in strings
internal void
JSON_escapechar(WrenVM * vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "value");
  char * value = wrenGetSlotString(vm, 1);
  char * result = value;

  /*
  "\0" // The NUL byte: 0.
  "\"" // A double quote character.
  "\\" // A backslash.
  "\a" // Alarm beep. (Who uses this?)
  "\b" // Backspace.
  "\f" // Formfeed.
  "\n" // Newline.
  "\r" // Carriage return.
  "\t" // Tab.
  "\v" // Vertical tab.
  */
  if(strcmp(value, "\0") == 0) {
    result = "\\0";
  } else if (strcmp(value, "\"") == 0) {
    result = "\\\"";
  } else if (strcmp(value, "\\") == 0) {
    result = "\\\\";
  } else if(strcmp(value, "\\") == 0) {
    result = "\\a";
  } else if (strcmp(value, "\b") == 0) {
    result = "\\b";
  } else if (strcmp(value, "\f") == 0) {
    result = "\\f";
  } else if (strcmp(value, "\n") == 0) {
    result = "\\n";
  } else if (strcmp(value, "\r") == 0) {
    result = "\\r";
  } else if (strcmp(value, "\t") == 0) {
    result = "\\t";
  } else if (strcmp(value, "\v") == 0) {
    result = "\\v";
  } else if (strcmp(value, "/") == 0) {
    // Escape / (solidus, slash)
    // https://stackoverflow.com/a/9735430
    // The feature of the slash escape allows JSON to be embedded in HTML (as SGML) and XML.
    // https://www.w3.org/TR/html4/appendix/notes.html#h-B.3.2
    // This is optional escaping. Maybe an option should be provided
    // to disable this escaping if not wanted.
    result = "\\/";
  }

  wrenSetSlotString(vm, 0, result);
}
