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
  uint32_t width = wrenGetSlotDouble(vm, 1);
  uint32_t height = wrenGetSlotDouble(vm, 2);
  ASSERT_SLOT_TYPE(vm, 1, NUM, "width");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "height");
  SDL_SetWindowSize(engine->window, width, height);
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
  bool value = wrenGetSlotBool(vm, 1);
  ENGINE_setupRenderer(engine, value);
}

internal void
WINDOW_setLockStep(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  engine->lockstep = wrenGetSlotBool(vm, 1);
}
