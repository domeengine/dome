// This informs the engine we want to stop running, and jumps to the end of the game loop if we have no errors to report.
void DOME_exit(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  engine->running = false;
  engine->exit_status = floor(wrenGetSlotDouble(vm, 1));
  if (engine->exit_status != 0) {
    wrenAbortFiber(vm, 1);
  } else {
    longjmp(loop_exit, EXIT_SUCCESS);
  }
}

