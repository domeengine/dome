typedef struct {
  bool complete;
  bool error;
  uint32_t id;
  WrenVM* vm;
  DBUFFER* buffer;
} ASYNCOP;

global_variable uint32_t idCount = 0;

internal void
ASYNCOP_allocate(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(ASYNCOP));
  op->vm = vm;
  op->id = idCount;
  idCount++;

  // We might want to hold onto a handle for this buffer
  op->buffer = wrenGetSlotForeign(vm, 1);
  op->complete = false;
  op->error = false;
}

internal void
ASYNCOP_finalize(void* data) {
  ASYNCOP* op = (ASYNCOP*)data;
}

internal void
ASYNCOP_getComplete(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, op->complete);
}

internal void
ASYNCOP_getResult(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  // HOW DO?
  wrenSetSlotHandle(vm, 0, op->handle);
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

typedef struct {
  ASYNCOP* op;
  DBUFFER* buffer;
  void* data;
  char name[256];
} TASK_DATA;

internal void
FILESYSTEM_load(WrenVM* vm) {
  // Thread: main
  INIT_TO_ZERO(ABC_TASK, task);
  TASK_DATA* taskData = malloc(sizeof(TASK_DATA));

  const char* path = wrenGetSlotString(vm, 1);
  strncpy(taskData->name, path, 255);
  taskData->name[255] = '\0';

  // TODO We probably need to hold a handle to the op
  taskData->op = (ASYNCOP*)wrenGetSlotForeign(vm, 2);

  taskData->buffer = (DBUFFER*)taskData->op->data;
  taskData->data = NULL;

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  task.type = TASK_LOAD_FILE;
  task.data = taskData;
  ABC_FIFO_pushTask(&engine->fifo, task);
}

internal void
FILESYSTEM_loadEventHandler(TASK_DATA* task) {
  // Thread: Async
  ASYNCOP* op = task->op;
  DBUFFER* buffer = task->buffer;

  char* fileData = readEntireFile(task->name, &buffer->length);

  SDL_Event event;
  SDL_memset(&event, 0, sizeof(event));
  event.type = ENGINE_EVENT_TYPE;
  event.user.code = EVENT_LOAD_FILE;
  event.user.data1 = op;
  // TODO: Use this to handle errors in future?
  event.user.data2 = fileData;
  SDL_PushEvent(&event);
  free(task);
}

internal void
FILESYSTEM_loadEventComplete(SDL_Event* event) {
  // Thread: Main
  ASYNCOP* op = event->user.data1;
  DBUFFER* buffer = op->buffer;
  buffer->data = event->user.data2;
  buffer->ready = true;
}

/*
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
*/
