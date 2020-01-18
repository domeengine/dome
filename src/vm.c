
internal WrenForeignClassMethods
VM_bind_foreign_class(WrenVM* vm, const char* module, const char* className) {
  WrenForeignClassMethods methods;
  // Assume an unknown class.
  methods.allocate = NULL;
  methods.finalize = NULL;

  #if DOME_OPT_FFI

  if (STRINGS_EQUAL(module, "ffi")) {
    if (STRINGS_EQUAL(className, "LibraryHandle")) {
      methods.allocate = LIBRARY_HANDLE_allocate;
      methods.finalize = LIBRARY_HANDLE_finalize;
    } else if (STRINGS_EQUAL(className, "Function")) {
      methods.allocate = FUNCTION_allocate;
      methods.finalize = FUNCTION_finalize;
    } else if (STRINGS_EQUAL(className, "StructTypeData")) {
      methods.allocate = STRUCT_TYPE_allocate;
      methods.finalize = STRUCT_TYPE_finalize;
    } else if (STRINGS_EQUAL(className, "Struct")) {
      methods.allocate = STRUCT_allocate;
      methods.finalize = STRUCT_finalize;
    } else if (STRINGS_EQUAL(className, "Pointer")) {
      methods.allocate = POINTER_allocate;
    }
  }
  #endif
  if (STRINGS_EQUAL(module, "image")) {
    if (STRINGS_EQUAL(className, "ImageData")) {
      methods.allocate = IMAGE_allocate;
      methods.finalize = IMAGE_finalize;
    }
  } else if (STRINGS_EQUAL(module, "io")) {
    if (STRINGS_EQUAL(className, "DataBuffer")) {
      methods.allocate = DBUFFER_allocate;
      methods.finalize = DBUFFER_finalize;
    } else if (STRINGS_EQUAL(className, "AsyncOperation")) {
      methods.allocate = ASYNCOP_allocate;
      methods.finalize = ASYNCOP_finalize;
    }
  } else if (STRINGS_EQUAL(module, "audio")) {
    if (STRINGS_EQUAL(className, "AudioData")) {
      methods.allocate = AUDIO_allocate;
      methods.finalize = AUDIO_finalize;
    } else if (STRINGS_EQUAL(className, "SystemChannel")) {
      methods.allocate = AUDIO_CHANNEL_allocate;
      methods.finalize = AUDIO_CHANNEL_finalize;
    }
  } else if (STRINGS_EQUAL(module, "input")) {
    if (STRINGS_EQUAL(className, "GamePad")) {
      methods.allocate = GAMEPAD_allocate;
      methods.finalize = GAMEPAD_finalize;
    }
  } else {
    // TODO: Check if it's a module we lazy-loaded

  }


  return methods;
}

internal WrenForeignMethodFn VM_bind_foreign_method(
    WrenVM* vm,
    const char* module,
    const char* className,
    bool isStatic,
    const char* signature) {

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ForeignFunctionMap fnMap = engine->fnMap;
  return MAP_get(&fnMap, module, className, signature, isStatic);
}

internal char* VM_load_module(WrenVM* vm, const char* name) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ModuleMap moduleMap = engine->moduleMap;
  if (strncmp("./", name, 2) != 0) {
    if (DEBUG_MODE) {
      printf("Loading module %s\n", name);
    }
    return (char*)ModuleMap_get(&moduleMap, name);
  }

  printf("Loading module %s\n", name);

  char* extension = ".wren";
  char* path;
  path = malloc(strlen(name)+strlen(extension)+1); /* make space for the new string (should check the return value ...) */
  strcpy(path, name); /* add the extension */
  strcat(path, extension); /* add the extension */

  // This pointer becomes owned by the WrenVM and freed later.
  char* file = ENGINE_readFile(engine, path, NULL);
  free(path);
  return file;
}

// Debug output for VM
internal void VM_write(WrenVM* vm, const char* text) {
  printf("%s", text);
}

internal void VM_error(WrenVM* vm, WrenErrorType type, const char* module,
    int line, const char* message) {

  if (DEBUG_MODE == false) {
    ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
    ModuleMap moduleMap = engine->moduleMap;

    if (module != NULL && ModuleMap_get(&moduleMap, module) != NULL) {
      return;
    }
  }

  if (type == WREN_ERROR_COMPILE) {
    printf("%s:%d: %s\n", module, line, message);
  } else if (type == WREN_ERROR_RUNTIME) {
    printf("Runtime error: %s\n", message);
  } else if (type == WREN_ERROR_STACK_TRACE) {
    printf("  %d: %s\n", line, module);
  }
}

internal WrenVM* VM_create(ENGINE* engine) {
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = VM_write;
  config.errorFn = VM_error;
  config.bindForeignMethodFn = VM_bind_foreign_method;
  config.bindForeignClassFn = VM_bind_foreign_class;
  config.loadModuleFn = VM_load_module;
  config.initialHeapSize = INITIAL_HEAP_SIZE;
  config.minHeapSize = 1024 * 1024 * 10;

  WrenVM* vm = wrenNewVM(&config);
  wrenSetUserData(vm, engine);

  // Set modules

  // DOME
  MAP_add(&engine->fnMap, "dome", "Process", "f_exit(_)", true, PROCESS_exit);
  MAP_add(&engine->fnMap, "dome", "Window", "resize(_,_)", true, WINDOW_resize);
  MAP_add(&engine->fnMap, "dome", "Window", "title=(_)", true, WINDOW_setTitle);
  MAP_add(&engine->fnMap, "dome", "Window", "vsync=(_)", true, WINDOW_setVsync);
  MAP_add(&engine->fnMap, "dome", "Window", "lockstep=(_)", true, WINDOW_setLockStep);
  MAP_add(&engine->fnMap, "dome", "Window", "title", true, WINDOW_getTitle);

#if DOME_OPT_FFI
  // FFI
  MAP_add(&engine->fnMap, "ffi", "Function", "f_call(_)", false, FUNCTION_call);
  MAP_add(&engine->fnMap, "ffi", "StructTypeData", "getMemberOffset(_)", false, STRUCT_TYPE_getOffset);
  MAP_add(&engine->fnMap, "ffi", "Struct", "getValue(_)", false, STRUCT_getValue);
  MAP_add(&engine->fnMap, "ffi", "Pointer", "asString()", false, POINTER_asString);
  MAP_add(&engine->fnMap, "ffi", "Pointer", "asBytes(_)", false, POINTER_asBytes);
  MAP_add(&engine->fnMap, "ffi", "Pointer", "reserve(_)", true, POINTER_reserve);
  MAP_add(&engine->fnMap, "ffi", "Pointer", "free()", false, POINTER_free);
#endif

  // Canvas
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_pset(_,_,_)", true, CANVAS_pset);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_rectfill(_,_,_,_,_)", true, CANVAS_rectfill);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_rect(_,_,_,_,_)", true, CANVAS_rect);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_line(_,_,_,_,_)", true, CANVAS_line);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_circle(_,_,_,_)", true, CANVAS_circle);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_circlefill(_,_,_,_)", true, CANVAS_circle_filled);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_ellipse(_,_,_,_,_)", true, CANVAS_ellipse);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_ellipsefill(_,_,_,_,_)", true, CANVAS_ellipsefill);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_print(_,_,_,_)", true, CANVAS_print);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_resize(_,_,_)", true, CANVAS_resize);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "width", true, CANVAS_getWidth);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "height", true, CANVAS_getHeight);

  // Image
  MAP_add(&engine->fnMap, "image", "ImageData", "draw(_,_)", false, IMAGE_draw);
  MAP_add(&engine->fnMap, "image", "ImageData", "width", false, IMAGE_getWidth);
  MAP_add(&engine->fnMap, "image", "ImageData", "height", false, IMAGE_getHeight);
  MAP_add(&engine->fnMap, "image", "ImageData", "drawArea(_,_,_,_,_,_)", false, IMAGE_drawArea);
  MAP_add(&engine->fnMap, "image", "ImageData", "transform(_)", false, IMAGE_transform);

  // Audio
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "enabled=(_)", false, AUDIO_CHANNEL_setEnabled);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "enabled", false, AUDIO_CHANNEL_getEnabled);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "loop=(_)", false, AUDIO_CHANNEL_setLoop);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "loop", false, AUDIO_CHANNEL_getLoop);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "pan=(_)", false, AUDIO_CHANNEL_setPan);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "pan", false, AUDIO_CHANNEL_getPan);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "volume", false, AUDIO_CHANNEL_getVolume);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "volume=(_)", false, AUDIO_CHANNEL_setVolume);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "state=(_)", false, AUDIO_CHANNEL_setState);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "audio=(_)", false, AUDIO_CHANNEL_setAudio);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "state", false, AUDIO_CHANNEL_getState);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "length", false, AUDIO_CHANNEL_getPosition);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "position", false, AUDIO_CHANNEL_getPosition);
  MAP_add(&engine->fnMap, "audio", "SystemChannel", "soundId", false, AUDIO_CHANNEL_getSoundId);

  MAP_add(&engine->fnMap, "audio", "AudioData", "length", false, AUDIO_getLength);

  MAP_add(&engine->fnMap, "audio", "AudioEngine", "f_update(_)", true, AUDIO_ENGINE_update);
  MAP_add(&engine->fnMap, "audio", "AudioEngine", "f_captureVariable()", true, AUDIO_ENGINE_capture);

  // FileSystem
  MAP_add(&engine->fnMap, "io", "FileSystem", "f_load(_,_)", true, FILESYSTEM_loadAsync);
  MAP_add(&engine->fnMap, "io", "FileSystem", "load(_)", true, FILESYSTEM_loadSync);
  MAP_add(&engine->fnMap, "io", "FileSystem", "save(_,_)", true, FILESYSTEM_saveSync);

  // Buffer
  MAP_add(&engine->fnMap, "io", "DataBuffer", "f_capture()", true, DBUFFER_capture);
  MAP_add(&engine->fnMap, "io", "DataBuffer", "f_data", false, DBUFFER_getData);
  MAP_add(&engine->fnMap, "io", "DataBuffer", "ready", false, DBUFFER_getReady);
  MAP_add(&engine->fnMap, "io", "DataBuffer", "f_length", false, DBUFFER_getLength);

  // AsyncOperation
  MAP_add(&engine->fnMap, "io", "AsyncOperation", "result", false, ASYNCOP_getResult);
  MAP_add(&engine->fnMap, "io", "AsyncOperation", "complete", false, ASYNCOP_getComplete);

  // Input
  MAP_add(&engine->fnMap, "input", "Keyboard", "isKeyDown(_)", true, KEYBOARD_isKeyDown);
  MAP_add(&engine->fnMap, "input", "Mouse", "x", true, MOUSE_getX);
  MAP_add(&engine->fnMap, "input", "Mouse", "y", true, MOUSE_getY);
  MAP_add(&engine->fnMap, "input", "Mouse", "isButtonPressed(_)", true, MOUSE_isButtonPressed);
  MAP_add(&engine->fnMap, "input", "GamePad", "f_getGamePadIds()", true, GAMEPAD_getGamePadIds);
  MAP_add(&engine->fnMap, "input", "GamePad", "f_isButtonPressed(_)", false, GAMEPAD_isButtonPressed);
  MAP_add(&engine->fnMap, "input", "GamePad", "getTrigger(_)", false, GAMEPAD_getTrigger);
  MAP_add(&engine->fnMap, "input", "GamePad", "close()", false, GAMEPAD_close);
  MAP_add(&engine->fnMap, "input", "GamePad", "f_getAnalogStick(_)", false, GAMEPAD_getAnalogStick);
  MAP_add(&engine->fnMap, "input", "GamePad", "attached", false, GAMEPAD_isAttached);
  MAP_add(&engine->fnMap, "input", "GamePad", "name", false, GAMEPAD_getName);
  MAP_add(&engine->fnMap, "input", "GamePad", "id", false, GAMEPAD_getId);

  return vm;
}

internal void VM_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
