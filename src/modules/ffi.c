typedef struct {
  void* handle;
  char name[];
} MODULE_HANDLE;

internal void
MODULE_HANDLE_allocate(WrenVM* vm) {
  char* libraryName = wrenGetSlotString(vm, 1);
  void* handle = SDL_LoadObject(libraryName);
  if (handle == NULL) {
    wrenSetSlotString(vm, 1, SDL_GetError());
    wrenAbortFiber(vm, 1);
    return;
  }
  MODULE_HANDLE* module = (MODULE_HANDLE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(MODULE_HANDLE) + sizeof(char) * strlen(libraryName) + 1);

  module->handle = handle;
  strcpy(module->name, libraryName);

  // TODO: Register module in module map
  // and functionmap
}

internal void
MODULE_HANDLE_finalize(void* handle) {
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

typedef struct {

} FUNCTION;

internal void
FUNCTION_allocate(WrenVM* vm) {
  MODULE_HANDLE* handle = wrenGetSlotForeign(vm, 1);
  char* fnName = wrenGetSlotString(vm, 2);
  char* returnType = wrenGetSlotString(vm, 3);
  // We need to process the argTypeList


  FUNCTION* function = wrenSetSlotNewForeign(vm, 0, 0, sizeof(FUNCTION));
}

internal void
FUNCTION_finalize(void* function) {
  free(function);
}

internal void
FUNCTION_call(WrenVM* vm) {
  WrenType type = (WrenType)floor(wrenGetSlotDouble(vm, 2));
  printf("%s\n", DEBUG_printWrenType(type));
  char* fnName = wrenGetSlotString(vm, 1);
  MODULE_HANDLE* module = wrenGetSlotForeign(vm, 0);
  WrenForeignMethodFn method = (WrenForeignMethodFn)SDL_LoadFunction(module->handle, fnName);
  if (method == NULL) {
    wrenSetSlotString(vm, 1, SDL_GetError());
    wrenAbortFiber(vm, 1);
    return;
  }
  method(vm);
}
