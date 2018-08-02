typedef struct {
  bool complete;
  bool error;
  uint32_t id;
  WrenVM* vm;
  WrenHandle* data;
} ASYNCOP;
global_variable uint32_t idCount = 0;

internal void
ASYNCOP_allocate(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(ASYNCOP));
  op->vm = vm;
  op->id = idCount;
  idCount++;

  op->data = wrenGetSlotHandle(vm, 1);
  op->complete = false;
  op->error = false;
}

internal void
ASYNCOP_finalize(void* data) {
  ASYNCOP* op = (ASYNCOP*)data;
  wrenReleaseHandle(op->vm, op->data);
}

internal void
ASYNCOP_getComplete(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, op->complete);
}

internal void
ASYNCOP_getResult(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotHandle(vm, 0, op->data);
}

typedef struct {
  bool ready;
  size_t length;
  char* data;
} DBUFFER;

internal void
DBUFFER_allocate(WrenVM* vm) {
  DBUFFER* buffer = (DBUFFER*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(DBUFFER));
  buffer->data = NULL;
  buffer->length = 0;
  buffer->ready = false;
}

internal void
DBUFFER_finalize(void* data) {
  DBUFFER* buffer = (DBUFFER*) data;
  if (buffer->ready && buffer->data != NULL) {
    free(buffer->data);
  }
}

internal void
DBUFFER_getLength(WrenVM* vm) {
  DBUFFER* buffer = wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, buffer->length);
}

internal void
DBUFFER_getReady(WrenVM* vm) {
  DBUFFER* buffer = wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, buffer->ready);
}

internal void
DBUFFER_getData(WrenVM* vm) {
  DBUFFER* buffer = wrenGetSlotForeign(vm, 0);
  wrenSetSlotString(vm, 0, buffer->data);
}



internal void
GAMEFILE_allocate(WrenVM* vm) {
  GAMEFILE* data = (GAMEFILE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(GAMEFILE));
  data->ready = false;
  data->length = 0;

  const char* path = wrenGetSlotString(vm, 1);
  strncpy(data->name, path, 255);
  data->name[255] = '\0';
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  INIT_TO_ZERO(ABC_TASK, task);

  task.type = TASK_LOAD_FILE;
  task.data = data;
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
GAMEFILE_getLength(WrenVM* vm) {
  GAMEFILE* file = wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, file->length);
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
