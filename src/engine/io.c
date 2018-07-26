
internal void
GAMEFILE_allocate(WrenVM* vm) {
  GAMEFILE* data = (GAMEFILE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(GAMEFILE));
  data->ready = false;

  const char* path = wrenGetSlotString(vm, 1);
  strncpy(data->name, path, 255);
  data->name[255] = '\0';
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  INIT_TO_ZERO(ABC_TASK, task);

  task.type = TASK_LOAD_FILE;
  task.data = data;
  printf("allocate\n");
  ABC_FIFO_pushTask(&engine->fifo, task);
}

internal void
GAMEFILE_loadComplete(GAMEFILE* file, char* data) {
  file->data = data;
  file->ready = true;
}

internal void
GAMEFILE_getReady(WrenVM* vm) {
  GAMEFILE* file = wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, file->ready);
}

internal void
GAMEFILE_getData(WrenVM* vm) {
  GAMEFILE* file = wrenGetSlotForeign(vm, 0);
  if (file->ready) {
    wrenSetSlotString(vm, 0, file->data);
  } else {
    wrenSetSlotNull(vm, 0);
  }
}

internal void
GAMEFILE_finalize(void* data) {
  GAMEFILE* file = (GAMEFILE*) data;
  if (file->ready && file->data != NULL) {
    free(file->data);
  }
}
