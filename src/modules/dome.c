// This informs the engine we want to stop running, and jumps to the end of the game loop if we have no errors to report.
internal void
PROCESS_exit(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  engine->running = false;
  engine->exit_status = floor(wrenGetSlotDouble(vm, 1));
  if (engine->exit_status != 0) {
    wrenAbortFiber(vm, 1);
  } else {
    longjmp(loop_exit, EXIT_SUCCESS);
  }
}


typedef struct {
  void* handle;
  char name[];
} MODULE;

internal void
MODULE_allocate(WrenVM* vm) {
  char* moduleName = wrenGetSlotString(vm, 1);
  char* libraryName = wrenGetSlotString(vm, 2);
  void* handle = SDL_LoadObject(libraryName);
  if (handle == NULL) {
    wrenSetSlotString(vm, 1, SDL_GetError());
    wrenAbortFiber(vm, 1);
    return;
  }
  MODULE* module = (MODULE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(MODULE) + sizeof(char) * strlen(moduleName) + 1);

  module->handle = handle;
  strcpy(module->name, moduleName);

  // TODO: Register module in module map
  // and functionmap
}

internal void
MODULE_finalize(void* handle) {
  SDL_UnloadObject(handle);
}

internal char*
DEBUG_printWrenType(WrenType type) {
  switch (type) {
    case WREN_TYPE_BOOL: return "boolean"; break;
    case WREN_TYPE_NUM: return "number"; break;
    case WREN_TYPE_FOREIGN: return "foreign"; break;
    case WREN_TYPE_LIST: return "list"; break;
    case WREN_TYPE_NULL: return "Null"; break;
    case WREN_TYPE_STRING: return "string"; break;
    default: return "unknown";
  }
}

internal void
MODULE_call(WrenVM* vm) {
  WrenType type = (WrenType)floor(wrenGetSlotDouble(vm, 2));
  printf("%s\n", DEBUG_printWrenType(type));
  char* fnName = wrenGetSlotString(vm, 1);
  MODULE* module = wrenGetSlotForeign(vm, 0);
  WrenForeignMethodFn method = (WrenForeignMethodFn)SDL_LoadFunction(module->handle, fnName);
  if (method == NULL) {
    wrenSetSlotString(vm, 1, SDL_GetError());
    wrenAbortFiber(vm, 1);
    return;
  }
  method(vm);
}



