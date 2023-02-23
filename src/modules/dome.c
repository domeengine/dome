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
PROCESS_getArguments(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  size_t count = engine->argc;
  char** arguments = engine->argv;
  wrenEnsureSlots(vm, 2);
  wrenSetSlotNewList(vm, 0);
  for (size_t i = 0; i < count; i++) {
    wrenSetSlotString(vm, 1, arguments[i]);
    wrenInsertInList(vm, 0, i, 1);
  }
}

internal void
PROCESS_getErrorDialog(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  bool errorDialogEnabled = engine->debug.errorDialog;
  wrenSetSlotBool(vm, 0, errorDialogEnabled);
}

internal void
PROCESS_setErrorDialog(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "value");
  bool errorDialogEnabled = wrenGetSlotBool(vm, 1);
  engine->debug.errorDialog = errorDialogEnabled;
}

internal void
STRING_UTILS_toLowercase(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "string");
  int length;
  const char* str = wrenGetSlotBytes(vm, 1, &length);
  char* dest = calloc(length + 1, sizeof(char));
  utf8ncpy(dest, str, length);
  utf8lwr(dest);
  wrenSetSlotBytes(vm, 0, dest, length);
  free(dest);
}

internal void
STRING_UTILS_toUppercase(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "string");
  int length;
  const char* str = wrenGetSlotBytes(vm, 1, &length);
  char* dest = calloc(length + 1, sizeof(char));
  utf8ncpy(dest, str, length);
  utf8upr(dest);
  wrenSetSlotBytes(vm, 0, dest, length);
  free(dest);
}

internal void
WINDOW_resize(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "width");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "height");

#ifndef __EMSCRIPTEN__
  uint32_t width = wrenGetSlotDouble(vm, 1);
  uint32_t height = wrenGetSlotDouble(vm, 2);

  // Account for High DPI by comparing the current window size
  // to the renderer's client output size.
  // This is considered more accurate than SDL's built in methods.
  int32_t currentWinWidth, currentWinHeight;
  int32_t currentRenderWidth, currentRenderHeight;
  SDL_GetWindowSize(engine->window, &currentWinWidth, &currentWinHeight);
  SDL_GetRendererOutputSize(engine->renderer, &currentRenderWidth, &currentRenderHeight);

  double factorH = currentRenderWidth / currentWinWidth;
  double factorV = currentRenderHeight / currentWinHeight;

  SDL_SetWindowSize(engine->window, width / factorH, height / factorV);
  // Window may not have resized to the specified value because of
  // desktop restraints, but SDL doesn't check this.
  // We can fetch the final display size from the renderer output.
  int32_t newWidth, newHeight;
  SDL_GetRendererOutputSize(engine->renderer, &newWidth, &newHeight);
  SDL_SetWindowSize(engine->window, newWidth / factorH, newHeight/factorV);
#else
  SDL_SetWindowSize(engine->window, engine->canvas.width, engine->canvas.height);
#endif
  ENGINE_updateTextRegion(engine);
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
  const char* title = wrenGetSlotString(vm, 1);
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
WINDOW_getFps(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ENGINE_DEBUG* debug = &engine->debug;
  // Choose alpha depending on how fast or slow you want old averages to decay.
  // 0.9 is usually a good choice.
  double framesThisSecond = 1000.0 / (debug->elapsed+1);
  double alpha = debug->alpha;
  debug->avgFps = alpha * debug->avgFps + (1.0 - alpha) * framesThisSecond;
  wrenSetSlotDouble(vm, 0, debug->avgFps);
}

internal void
WINDOW_setColor(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "color");
  uint32_t color = (uint32_t)wrenGetSlotDouble(vm, 1);
  uint8_t r, g, b;
  getColorComponents(color, &r, &g, &b);
  SDL_SetRenderDrawColor(engine->renderer, r, g, b, 0xFF);
}

internal void
WINDOW_getColor(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  uint8_t r, g, b, a;
  SDL_GetRenderDrawColor(engine->renderer, &r, &g, &b, &a);
  uint32_t color = (b << 16) | (g << 8) | r;
  wrenSetSlotDouble(vm, 0, color);
}

internal void
WINDOW_setIntegerScale(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, BOOL, "scale");
  bool value = wrenGetSlotBool(vm, 1);
  SDL_RenderSetIntegerScale(engine->renderer, value);
}

internal void
WINDOW_getIntegerScale(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotBool(vm, 0, SDL_RenderGetIntegerScale(engine->renderer));
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
  wrenSetSlotBytes(vm, 0, version, len);
}

const char* LOG_LEVEL[] = {
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};
const char* LOG_COLOR[] = {
  "\033[36m",
  "\033[32m",
  "\033[33m",
  "\033[31m",
  "\033[31m"
};

internal void
LOG_print(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  uint8_t levelNum = mid(0, wrenGetSlotDouble(vm, 1), 4);
  if (levelNum < engine->logLevel) {
    return;
  }
  const char* level = LOG_LEVEL[levelNum];
  const char* color = LOG_COLOR[levelNum];
  const char* line = wrenGetSlotString(vm, 2);
  const char* context = NULL;
  if (wrenGetSlotType(vm, 3) == WREN_TYPE_STRING) {
    context = wrenGetSlotString(vm, 3);
  }
  bool tty = isatty(STDOUT_FILENO);
  size_t length = engine->padding;
  const size_t lineLength = (context == NULL ? 0 : 3 + strlen(context)) + strlen(level);

  if (lineLength > length) {
    length = engine->padding = lineLength;
  }

  size_t padding;
  padding = length - lineLength + 1;

  if (tty) {
    printf("%s", color);
  }
  printf("[%s]", level);
  if (context != NULL) {
    printf(" [%s]", context);
  }
  if (tty) {
    printf("\033[0m");
  }
  printf(":");
  for (size_t i = 0; i < padding; i++) {
    printf(" ");
  }
  printf("%s\n", line);

  char* start;
  char* out;
  start = out = (char*)malloc(strlen(line) + length + 7);
  out += sprintf(out, "[%s]", level);
  if (context != NULL) {
    out += sprintf(out, " [%s]", context);
  }
  out += sprintf(out, ":");
  for (size_t i = 0; i < padding; i++) {
    out += sprintf(out, " ");
  }
  out += sprintf(out, "%s", line);
  *out = '\0';
  ENGINE_writeToLogFile(engine, start);
  ENGINE_writeToLogFile(engine, "\n");

  if (levelNum == 4) {
    VM_ABORT(vm, start);
  }
  free(start);
}

internal void
LOG_setLevel(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ASSERT_SLOT_TYPE(vm, 1, STRING, "log level");
  const char* str = wrenGetSlotString(vm, 1);
  int i = 0;
  for (i = 0; i < 5; i++) {
    if (STRINGS_EQUAL(str, LOG_LEVEL[i])) {
      engine->logLevel = i;
      break;
    }
  }
}

internal void
LOG_getLevel(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotString(vm, 0, LOG_LEVEL[engine->logLevel]);
}
