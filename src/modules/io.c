typedef struct {
  bool ready;
  size_t length;
  char* data;
} DBUFFER;

typedef struct {
  bool complete;
  bool error;
  WrenVM* vm;
  WrenHandle* bufferHandle;
} ASYNCOP;


internal void
ASYNCOP_allocate(WrenVM* vm) {
  // Get a handle to the Stat class. We'll hang on to this so we don't have to
  // look it up by name every time.
  wrenEnsureSlots(vm, 2);
  wrenSetSlotHandle(vm, 1, bufferClass);
  DBUFFER* buffer = (DBUFFER*)wrenSetSlotNewForeign(vm, 1, 1, sizeof(DBUFFER));
  buffer->data = NULL;
  buffer->length = 0;
  buffer->ready = false;

  ASYNCOP* op = (ASYNCOP*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(ASYNCOP));
  op->vm = vm;

  // This is a handle to our specific buffer, not the DataBuffer class
  op->bufferHandle = wrenGetSlotHandle(vm, 1);
  op->complete = false;
  op->error = false;
}

internal void
ASYNCOP_finalize(void* data) {
  ASYNCOP* op = data;
  wrenReleaseHandle(op->vm, op->bufferHandle);
}

internal void
ASYNCOP_getComplete(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, op->complete);
}

internal void
ASYNCOP_getResult(WrenVM* vm) {
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, op->bufferHandle);
}

internal void
DBUFFER_capture(WrenVM* vm) {
  if (bufferClass == NULL) {
    wrenGetVariable(vm, "io", "DataBuffer", 0);
    bufferClass = wrenGetSlotHandle(vm, 0);
  }
}

internal void
DBUFFER_allocate(WrenVM* vm) {
  wrenEnsureSlots(vm, 1);
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
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, buffer->length);
}

internal void
DBUFFER_getReady(WrenVM* vm) {
  DBUFFER* buffer = wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBool(vm, 0, buffer->ready);
}

internal void
DBUFFER_getData(WrenVM* vm) {
  DBUFFER* buffer = wrenGetSlotForeign(vm, 0);
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBytes(vm, 0, buffer->data, buffer->length);
}

typedef struct {
  WrenVM* vm;
  WrenHandle* opHandle;
  WrenHandle* bufferHandle;
  char name[256];
  size_t length;
  char* buffer;
} TASK_DATA;

internal void
FILESYSTEM_loadAsync(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "file path");
  // Thread: main
  INIT_TO_ZERO(ABC_TASK, task);
  TASK_DATA* taskData = malloc(sizeof(TASK_DATA));

  const char* path = wrenGetSlotString(vm, 1);
  strncpy(taskData->name, path, 255);
  taskData->name[255] = '\0';

  // TODO We probably need to hold a handle to the op
  taskData->vm = vm;
  taskData->opHandle = wrenGetSlotHandle(vm, 2);
  ASYNCOP* op = wrenGetSlotForeign(vm, 2);
  taskData->bufferHandle = op->bufferHandle;
  taskData->buffer = NULL;
  taskData->length = 0;

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  task.type = TASK_LOAD_FILE;
  task.data = taskData;
  ABC_FIFO_pushTask(&engine->fifo, task);
}

internal void
FILESYSTEM_loadEventHandler(void* data) {
  TASK_DATA* task = data;

  // Thread: Async
  ENGINE* engine = (ENGINE*)wrenGetUserData(task->vm);
  task->buffer = ENGINE_readFile(engine, task->name, &task->length, NULL);

  SDL_Event event;
  SDL_memset(&event, 0, sizeof(event));
  event.type = ENGINE_EVENT_TYPE;
  event.user.code = EVENT_LOAD_FILE;
  event.user.data1 = task;
  // TODO: Use this to handle errors in future?
  event.user.data2 = NULL;
  SDL_PushEvent(&event);
}

internal void
FILESYSTEM_saveSync(WrenVM* vm) {
  int length;
  ASSERT_SLOT_TYPE(vm, 1, STRING, "file path");
  ASSERT_SLOT_TYPE(vm, 2, STRING, "file data");
  const char* path = wrenGetSlotString(vm, 1);
  const char* data = wrenGetSlotBytes(vm, 2, &length);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ENGINE_WRITE_RESULT result = ENGINE_writeFile(engine, path, data, length);
  if (result == ENGINE_WRITE_PATH_INVALID) {
    size_t len = 22 + strlen(path);
    char message[len];
    snprintf(message, len, "Could not find file: %s", path);
    VM_ABORT(vm, message);
    return;
  }
}

internal void
FILESYSTEM_loadSync(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "file path");
  const char* path = wrenGetSlotString(vm, 1);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  size_t length;
  char* data = ENGINE_readFile(engine, path, &length, NULL);
  if (data == NULL) {
    size_t len = 22 + strlen(path);
    char message[len];
    snprintf(message, len, "Could not find file: %s", path);
    VM_ABORT(vm, message);
    return;
  }
  wrenEnsureSlots(vm, 1);
  wrenSetSlotBytes(vm, 0, data, length);
  free(data);
}

internal void
FILESYSTEM_loadEventComplete(SDL_Event* event) {
  // Thread: Main
  TASK_DATA* task = event->user.data1;
  WrenVM* vm = task->vm;
  wrenEnsureSlots(vm, 4);

  wrenSetSlotHandle(vm, 1, task->opHandle);
  ASYNCOP* op = (ASYNCOP*)wrenGetSlotForeign(vm, 1);
  wrenSetSlotHandle(vm, 2, task->bufferHandle);
  DBUFFER* buffer = (DBUFFER*)wrenGetSlotForeign(vm, 2);

  buffer->data = task->buffer;
  buffer->length = task->length;
  buffer->ready = true;

  op->complete = true;

  // Free resources and handles
  wrenReleaseHandle(vm, task->opHandle);
  free(task);
}

internal void
FILESYSTEM_listFiles(WrenVM* vm) {
  const char* path = wrenGetSlotString(vm, 1);
  bool shouldFree;
  const char* fullPath = resolvePath(path, &shouldFree);
  tinydir_dir dir;
  int result = tinydir_open(&dir, fullPath);
  if (shouldFree) {
    free((void*)fullPath);
  }
  if (result == -1) {
    VM_ABORT(vm, "Directory could not be opened");
  } else {
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    while (dir.has_next) {
      tinydir_file file;
      tinydir_readfile(&dir, &file);
      if (!file.is_dir)
      {
        // Only files
        wrenSetSlotString(vm, 1, file.name);
        // Append slot 1 to the list in slot 0
        wrenInsertInList(vm, 0, -1, 1);
      }
      tinydir_next(&dir);
    }
  }

  tinydir_close(&dir);
}

internal void
FILESYSTEM_listDirectories(WrenVM* vm) {
  const char* path = wrenGetSlotString(vm, 1);
  bool shouldFree;
  const char* fullPath = resolvePath(path, &shouldFree);
  tinydir_dir dir;
  int result = tinydir_open(&dir, fullPath);
  if (shouldFree) {
    free((void*)fullPath);
  }
  if (result == -1) {
    VM_ABORT(vm, "Directory could not be opened");
  } else {
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    while (dir.has_next) {
      tinydir_file file;
      tinydir_readfile(&dir, &file);
      if (file.is_dir)
      {
        // Only directories
        wrenSetSlotString(vm, 1, file.name);
        // Append slot 1 to the list in slot 0
        wrenInsertInList(vm, 0, -1, 1);
      }
      tinydir_next(&dir);
    }
  }

  tinydir_close(&dir);
}

internal void
FILESYSTEM_getPrefPath(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "organisation");
  ASSERT_SLOT_TYPE(vm, 2, STRING, "application name");
  const char* org = wrenGetSlotString(vm, 1);
  const char* app = wrenGetSlotString(vm, 2);
  char* path = SDL_GetPrefPath(org, app);
  if (path != NULL) {
    wrenSetSlotString(vm, 0, path);
    SDL_free(path);
  } else {
    wrenSetSlotString(vm, 0, BASEPATH_get());
  }
}

internal void
FILESYSTEM_getBasePath(WrenVM* vm) {
  wrenSetSlotString(vm, 0, BASEPATH_get());
}

internal void
FILESYSTEM_createDirectory(WrenVM *vm) {
  const char* path = wrenGetSlotString(vm, 1);
  mode_t mode = 0777;

  bool shouldFree;
  const char* fullPath = resolvePath(path, &shouldFree);
  int result = mkdirp(fullPath, mode);
  if (shouldFree) {
    free((void*)fullPath);
  }
  if (result == -1) {
    VM_ABORT(vm, "Directory could not be created");
  }
}

internal void
FILESYSTEM_doesFileExist(WrenVM* vm)
{
 ASSERT_SLOT_TYPE(vm, 1, STRING, "file path");
 const char* path = wrenGetSlotString(vm, 1);
 ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
 
 bool fileCheck = ENGINE_fileExists(engine, path);
 
 if (!fileCheck) {
   wrenSetSlotBool(vm, 0, false);
   return;
 }
 wrenSetSlotBool(vm, 0, true);
 return;
}

internal void
FILESYSTEM_doesDirectoryExist(WrenVM* vm)
{
 ASSERT_SLOT_TYPE(vm, 1, STRING, "dir path");
 const char* path = wrenGetSlotString(vm, 1);
 ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
 
 int dirCheck = ENGINE_directoryExists(engine, path);
 wrenSetSlotDouble(vm, 0, (double)dirCheck);
 return;
}
