internal WrenForeignClassMethods
VM_bind_foreign_class(WrenVM* vm, const char* module, const char* className) {
  WrenForeignClassMethods methods;
  // Assume an unknown class.
  methods.allocate = NULL;
  methods.finalize = NULL;

  if (strcmp(module, "graphics") == 0) {
    if (strcmp(className, "ImageData") == 0) {
      methods.allocate = IMAGE_allocate;
      methods.finalize = IMAGE_finalize;
    }
  }

  if (strcmp(module, "io") == 0) {
    if (strcmp(className, "DataBuffer") == 0) {
      methods.allocate = DBUFFER_allocate;
      methods.finalize = DBUFFER_finalize;
    } else if (strcmp(className, "AsyncOperation") == 0) {
      methods.allocate = ASYNCOP_allocate;
      methods.finalize = ASYNCOP_finalize;
    }
  }

  if (strcmp(module, "audio") == 0) {
    if (strcmp(className, "AudioData") == 0) {
      methods.allocate = AUDIO_allocate;
      methods.finalize = AUDIO_finalize;
    } else if (strcmp(className, "AudioChannel") == 0) {
      methods.allocate = AUDIO_CHANNEL_allocate;
      methods.finalize = AUDIO_CHANNEL_finalize;
    } else if (strcmp(className, "AudioEngineImpl") == 0) {
      methods.allocate = AUDIO_ENGINE_allocate;
      methods.finalize = AUDIO_ENGINE_finalize;
    }
  }

  if (strcmp(module, "point") == 0) {
    if (strcmp(className, "Point") == 0) {
      methods.allocate = POINT_allocate;
      methods.finalize = POINT_finalize;
    }
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
  printf("Loading module %s\n", name);
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ModuleMap moduleMap = engine->moduleMap;
  if (strncmp("./", name, 2) != 0) {
    return (char*)ModuleMap_get(&moduleMap, name);
  }

  char* extension = ".wren";
  char* path;
  path = malloc(strlen(name)+strlen(extension)+1); /* make space for the new string (should check the return value ...) */
  strcpy(path, name); /* add the extension */
  strcat(path, extension); /* add the extension */

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

  WrenVM* vm = wrenNewVM(&config);
  wrenSetUserData(vm, engine);

  // Set modules

  // DOME
  MAP_add(&engine->fnMap, "dome", "DOME", "f_exit(_)", true, DOME_exit);
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
  MAP_add(&engine->fnMap, "graphics", "ImageData", "draw(_,_)", false, IMAGE_draw);
  MAP_add(&engine->fnMap, "graphics", "ImageData", "width", false, IMAGE_getWidth);
  MAP_add(&engine->fnMap, "graphics", "ImageData", "height", false, IMAGE_getHeight);
  MAP_add(&engine->fnMap, "graphics", "ImageData", "drawArea(_,_,_,_,_,_)", false, IMAGE_drawArea);

  // Audio
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "enabled=(_)", false, AUDIO_CHANNEL_setEnabled);
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "loop=(_)", false, AUDIO_CHANNEL_setLoop);
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "pan=(_)", false, AUDIO_CHANNEL_setPan);
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "volume=(_)", false, AUDIO_CHANNEL_setVolume);
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "isFinished", false, AUDIO_CHANNEL_isFinished);
  MAP_add(&engine->fnMap, "audio", "AudioChannel", "id", false, AUDIO_CHANNEL_getId);
  MAP_add(&engine->fnMap, "audio", "AudioData", "unload()", false, AUDIO_unload);
  MAP_add(&engine->fnMap, "audio", "AudioEngineImpl", "f_update(_)", false, AUDIO_ENGINE_update);

  // FileSystem
  MAP_add(&engine->fnMap, "io", "FileSystem", "f_load(_,_)", true, FILESYSTEM_load);
  MAP_add(&engine->fnMap, "io", "FileSystem", "loadSync(_)", true, FILESYSTEM_loadSync);

  // Buffer
  MAP_add(&engine->fnMap, "io", "DataBuffer", "f_data", false, DBUFFER_getData);
  MAP_add(&engine->fnMap, "io", "DataBuffer", "ready", false, DBUFFER_getReady);
  MAP_add(&engine->fnMap, "io", "DataBuffer", "f_length", false, DBUFFER_getLength);

  // AsyncOperation
  MAP_add(&engine->fnMap, "io", "AsyncOperation", "result", false, ASYNCOP_getResult);
  MAP_add(&engine->fnMap, "io", "AsyncOperation", "complete", false, ASYNCOP_getComplete);

  // Input
  MAP_add(&engine->fnMap, "input", "Keyboard", "isKeyDown(_)", true, KEYBOARD_isKeyDown);

  // Point
  MAP_add(&engine->fnMap, "point", "Point", "x", false, POINT_getX);
  MAP_add(&engine->fnMap, "point", "Point", "y", false, POINT_getY);
  MAP_add(&engine->fnMap, "point", "Point", "x=(_)", false, POINT_setX);
  MAP_add(&engine->fnMap, "point", "Point", "y=(_)", false, POINT_setY);
  return vm;
}

internal void VM_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
