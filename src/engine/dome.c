// This informs the engine we want to stop running, but we have to wait until
// we reach the main loop to quit.
void DOME_exit(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  engine->running = false;
  engine->exit_status = floor(wrenGetSlotDouble(vm, 1));
  if (engine->exit_status != 0) {
    wrenAbortFiber(vm, 1);
  }
}

