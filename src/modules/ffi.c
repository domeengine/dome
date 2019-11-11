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
MODULE_HANDLE_finalize(void* ptr) {
  MODULE_HANDLE* handle = ptr;
  printf("SDL Unload Module: %s", handle->name);
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
  ffi_cif cif;
  void* methodPtr;
} FUNCTION;

ffi_type* toFFIType(char* name) {
  if (STRINGS_EQUAL(name, "void")) {
    return &ffi_type_void;
  } else if (STRINGS_EQUAL(name, "uint")) {
    return &ffi_type_uint;
  } else if (STRINGS_EQUAL(name, "sint")) {
    return &ffi_type_sint;
  } else if (STRINGS_EQUAL(name, "pointer")) {
    return &ffi_type_pointer;
    // TODO: handle other types

  } else {
    // TODO: handle structs
    return &ffi_type_pointer;
  }
}

internal void
FUNCTION_allocate(WrenVM* vm) {
  MODULE_HANDLE* module = wrenGetSlotForeign(vm, 1);
  char* fnName = wrenGetSlotString(vm, 2);
  char* returnType = wrenGetSlotString(vm, 3);
  // TODO: Variadic functions
  FUNCTION* function = wrenSetSlotNewForeign(vm, 0, 0, sizeof(FUNCTION));
  function->methodPtr = SDL_LoadFunction(module->handle, fnName);
  if (function->methodPtr == NULL) {
    wrenSetSlotString(vm, 1, "Could not bind to function");
    wrenAbortFiber(vm, 1);
  }

  int argCount = wrenGetListCount(vm, 4);
  ffi_type* retType = toFFIType(returnType);

  ffi_type* argTypes[argCount];
  for (int i = 0; i < argCount; i++) {
    // Move element i from List to slot 3
    wrenGetListElement(vm, 4, i, 3);
    char* typeName = wrenGetSlotString(vm, 3);
    printf("%s\n", typeName);
    argTypes[i] = toFFIType(typeName);
  }

  ffi_status result = ffi_prep_cif(&function->cif, FFI_DEFAULT_ABI, argCount, retType, argTypes);

  if (result == FFI_BAD_TYPEDEF) {
    wrenSetSlotString(vm, 1, "Invalid FFI Typedef");
    wrenAbortFiber(vm, 1);
  } else if (result == FFI_BAD_ABI) {
    wrenSetSlotString(vm, 1, "Invalid FFI ABI");
    wrenAbortFiber(vm, 1);
  }
}

internal void
FUNCTION_finalize(void* function) {
  // free(function);
}

internal void
FUNCTION_call(WrenVM* vm) {
  wrenEnsureSlots(vm, 3);
  FUNCTION* function = wrenGetSlotForeign(vm, 0);
  unsigned int argCount = wrenGetListCount(vm, 1);
  void* args[argCount];
  ffi_type** argTypes = function->cif.arg_types;

  if (argCount != function->cif.nargs) {
    wrenSetSlotString(vm, 1, "FFI: Argument mismatch");
    wrenAbortFiber(vm, 1);
    return;
  }

  for (unsigned int i = 0; i < argCount; i++) {
    // Move element i from List to slot 3
    wrenGetListElement(vm, 1, i, 2);
    args[i] = alloca(sizeof(argTypes[i]->size));
    // TODO: Cast to type and store

    int* value = args[i];
    *value = (int)wrenGetSlotDouble(vm, 2);
    printf("arg; %i\n", *value);
  }

  void* returnValue = alloca(function->cif.rtype->size);
  ffi_call(&(function->cif), FFI_FN(function->methodPtr), returnValue, args);

  ffi_type* returnType = function->cif.rtype;
  switch (returnType->type) {
    case FFI_TYPE_UINT8: {
      uint8_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_SINT8: {
      int8_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_UINT16: {
      uint16_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_SINT16: {
      int16_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_UINT32: {
      uint32_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_SINT32: {
      int32_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_UINT64: {
      uint64_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_SINT64: {
      int64_t* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_FLOAT: {
      float* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_DOUBLE: {
      double* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_LONGDOUBLE: {
      long double* ptr = returnValue;
      wrenSetSlotDouble(vm, 0, *ptr);
    } break;
    case FFI_TYPE_VOID:
    default: wrenSetSlotNull(vm, 0); break;
  }
}
