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
    } else if (STRINGS_EQUAL(className, "DrawCommand")) {
      methods.allocate = DRAW_COMMAND_allocate;
      methods.finalize = DRAW_COMMAND_finalize;
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
  } else if (STRINGS_EQUAL(module, "font")) {
    if (STRINGS_EQUAL(className, "FontFile")) {
      methods.allocate = FONT_allocate;
      methods.finalize = FONT_finalize;
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

  // This file is seperate because it has a Copyright notice with it.
#include "signature.c.inc"

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  MAP moduleMap = engine->moduleMap;
  return MAP_getFunction(&moduleMap, module, fullName);
}

internal char* VM_load_module(WrenVM* vm, const char* name) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  MAP moduleMap = engine->moduleMap;
  if (strncmp("./", name, 2) != 0) {
    if (DEBUG_MODE) {
      ENGINE_printLog(engine, "Loading module %s\n", name);
    }
    return (char*)MAP_getSource(&moduleMap, name);
  }

  ENGINE_printLog(engine, "Loading module %s\n", name);

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
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ENGINE_printLog(engine, "%s", text);
}

internal void VM_error(WrenVM* vm, WrenErrorType type, const char* module,
    int line, const char* message) {

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);

  if (DEBUG_MODE == false) {
    MAP moduleMap = engine->moduleMap;

    if (module != NULL && MAP_getModule(&moduleMap, module) != NULL) {
      return;
    }
  }


  size_t bufSize = 255;
  char error[bufSize];

  if (type == WREN_ERROR_COMPILE) {
    snprintf(error, bufSize, "%s:%d: %s\n", module, line, message);
  } else if (type == WREN_ERROR_RUNTIME) {
    snprintf(error, bufSize, "Runtime error: %s\n", message);
  } else if (type == WREN_ERROR_STACK_TRACE) {
    snprintf(error, bufSize, "  %d: %s\n", line, module);
  }
  size_t len = strlen(error);
  while ((len + engine->debug.errorBufLen) >= engine->debug.errorBufMax) {
    char* oldBuf = engine->debug.errorBuf;
    engine->debug.errorBufMax += 64;
    engine->debug.errorBuf = realloc(engine->debug.errorBuf, sizeof(char) * engine->debug.errorBufMax);
    if (engine->debug.errorBufMax == 64) {
      engine->debug.errorBuf[0] = '\0';
    }
    if (engine->debug.errorBuf == NULL) {
      // If we can't allocate more memory, rollback to the old pointer.
      engine->debug.errorBuf = oldBuf;
      engine->debug.errorBufMax -= 64;
      return;
    }
  }
  strcat(engine->debug.errorBuf, error);
  engine->debug.errorBufLen += len;
}

internal WrenVM* VM_create(ENGINE* engine) {
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = VM_write;
  config.errorFn = VM_error;
  config.bindForeignMethodFn = VM_bind_foreign_method;
  config.bindForeignClassFn = VM_bind_foreign_class;
  config.loadModuleFn = VM_load_module;

  WrenVM* vm = wrenNewVM(&config);
  wrenSetUserData(vm, engine);

  // Set modules

  // DOME
  MAP_addFunction(&engine->moduleMap, "dome", "static Process.f_exit(_)", PROCESS_exit);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.resize(_,_)", WINDOW_resize);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.title=(_)", WINDOW_setTitle);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.vsync=(_)", WINDOW_setVsync);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.lockstep=(_)", WINDOW_setLockStep);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.title", WINDOW_getTitle);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.fullscreen=(_)", WINDOW_setFullscreen);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.fullscreen", WINDOW_getFullscreen);

#if DOME_OPT_FFI
  // FFI
  MAP_addFunction(&engine->moduleMap, "ffi", "Function.f_call(_)", FUNCTION_call);
  MAP_addFunction(&engine->moduleMap, "ffi", "StructTypeData.getMemberOffset(_)", STRUCT_TYPE_getOffset);
  MAP_addFunction(&engine->moduleMap, "ffi", "Struct.getValue(_)", STRUCT_getValue);
  MAP_addFunction(&engine->moduleMap, "ffi", "Pointer.asString()", POINTER_asString);
  MAP_addFunction(&engine->moduleMap, "ffi", "Pointer.asBytes(_)", POINTER_asBytes);
  MAP_addFunction(&engine->moduleMap, "ffi", "static Pointer.reserve(_)", POINTER_reserve);
  MAP_addFunction(&engine->moduleMap, "ffi", "Pointer.free()", POINTER_free);
#endif

  // Canvas
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_pset(_,_,_)", CANVAS_pset);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_rectfill(_,_,_,_,_)", CANVAS_rectfill);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_rect(_,_,_,_,_)", CANVAS_rect);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_line(_,_,_,_,_)", CANVAS_line);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_circle(_,_,_,_)", CANVAS_circle);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_circlefill(_,_,_,_)", CANVAS_circle_filled);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_ellipse(_,_,_,_,_)", CANVAS_ellipse);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_ellipsefill(_,_,_,_,_)", CANVAS_ellipsefill);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_print(_,_,_,_)", CANVAS_print);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_resize(_,_,_)", CANVAS_resize);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.width", CANVAS_getWidth);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.height", CANVAS_getHeight);

  // Image
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.draw(_,_)", IMAGE_draw);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.width", IMAGE_getWidth);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.height", IMAGE_getHeight);
  MAP_addFunction(&engine->moduleMap, "image", "DrawCommand.draw(_,_)", DRAW_COMMAND_draw);

  // Audio
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.enabled=(_)", AUDIO_CHANNEL_setEnabled);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.enabled", AUDIO_CHANNEL_getEnabled);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.loop=(_)", AUDIO_CHANNEL_setLoop);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.loop", AUDIO_CHANNEL_getLoop);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.pan=(_)", AUDIO_CHANNEL_setPan);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.pan", AUDIO_CHANNEL_getPan);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.volume", AUDIO_CHANNEL_getVolume);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.volume=(_)", AUDIO_CHANNEL_setVolume);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.state=(_)", AUDIO_CHANNEL_setState);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.audio=(_)", AUDIO_CHANNEL_setAudio);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.state", AUDIO_CHANNEL_getState);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.length", AUDIO_CHANNEL_getPosition);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.position", AUDIO_CHANNEL_getPosition);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.soundId", AUDIO_CHANNEL_getSoundId);

  MAP_addFunction(&engine->moduleMap, "audio", "AudioData.length", AUDIO_getLength);

  MAP_addFunction(&engine->moduleMap, "audio", "static AudioEngine.f_update(_)", AUDIO_ENGINE_update);
  MAP_addFunction(&engine->moduleMap, "audio", "static AudioEngine.f_captureVariable()", AUDIO_ENGINE_capture);

  // FileSystem
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.f_load(_,_)", FILESYSTEM_loadAsync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.load(_)", FILESYSTEM_loadSync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.save(_,_)", FILESYSTEM_saveSync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.listFiles(_)", FILESYSTEM_listFiles);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.listDirectories(_)", FILESYSTEM_listDirectories);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.prefPath(_,_)", FILESYSTEM_getPrefPath);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.basePath()", FILESYSTEM_getBasePath);


  // Buffer
  MAP_addFunction(&engine->moduleMap, "io", "static DataBuffer.f_capture()", DBUFFER_capture);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.f_data", DBUFFER_getData);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.ready", DBUFFER_getReady);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.f_length", DBUFFER_getLength);

  // AsyncOperation
  MAP_addFunction(&engine->moduleMap, "io", "AsyncOperation.result", ASYNCOP_getResult);
  MAP_addFunction(&engine->moduleMap, "io", "AsyncOperation.complete", ASYNCOP_getComplete);

  // Input
  MAP_addFunction(&engine->moduleMap, "input", "static Keyboard.isKeyDown(_)", KEYBOARD_isKeyDown);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.x", MOUSE_getX);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.y", MOUSE_getY);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.isButtonPressed(_)", MOUSE_isButtonPressed);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.hidden=(_)", MOUSE_setHidden);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.hidden", MOUSE_getHidden);
  MAP_addFunction(&engine->moduleMap, "input", "static GamePad.f_getGamePadIds()", GAMEPAD_getGamePadIds);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.f_isButtonPressed(_)", GAMEPAD_isButtonPressed);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.getTrigger(_)", GAMEPAD_getTrigger);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.close()", GAMEPAD_close);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.f_getAnalogStick(_)", GAMEPAD_getAnalogStick);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.attached", GAMEPAD_isAttached);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.name", GAMEPAD_getName);
  MAP_addFunction(&engine->moduleMap, "input", "GamePad.id", GAMEPAD_getId);

  return vm;
}

internal void VM_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
