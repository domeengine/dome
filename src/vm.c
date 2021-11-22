internal WrenForeignClassMethods
VM_bind_foreign_class(WrenVM* vm, const char* moduleName, const char* className) {
  WrenForeignClassMethods methods;
  // Assume an unknown class.
  methods.allocate = NULL;
  methods.finalize = NULL;

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  MAP moduleMap = engine->moduleMap;
  MAP_getClassMethods(&moduleMap, moduleName, className, &methods);

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

internal WrenLoadModuleResult
VM_load_module(WrenVM* vm, const char* name) {
  WrenLoadModuleResult result = { 0 };

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  MAP moduleMap = engine->moduleMap;

  if (DEBUG_MODE) {
    ENGINE_printLog(engine, "Loading module %s from ", name);
  }

  // Check against wren optional modules
#if WREN_OPT_META
  if (strcmp(name, "meta") == 0) {
    if (DEBUG_MODE) {
      ENGINE_printLog(engine, "wren\n", name);
    }
    return result;
  }
#endif
#if WREN_OPT_RANDOM
  if (strcmp(name, "random") == 0) {
    if (DEBUG_MODE) {
      ENGINE_printLog(engine, "wren\n", name);
    }
    return result;
  }
#endif

  // Check against dome modules
  char* module = (char*)MAP_getSource(&moduleMap, name);

  if (module != NULL) {
    if (DEBUG_MODE) {
      ENGINE_printLog(engine, "dome\n", name);
    }
    result.source = module;
    return result;
  }

  // Otherwise, search on filesystem
  char* extension = ".wren";
  char* path;
  path = malloc(strlen(name)+strlen(extension)+1); /* make space for the new string (should check the return value ...) */
  strcpy(path, name); /* add the extension */
  strcat(path, extension); /* add the extension */

  if (DEBUG_MODE) {
    ENGINE_printLog(engine, "%s\n", engine->tar ? "egg bundle" : "filesystem");
  }

  // This pointer becomes owned by the WrenVM and freed later.
  char* file = ENGINE_readFile(engine, path, NULL);
  free(path);

  result.source = file;
  return result;
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

internal const char*
VM_resolve_module_name(WrenVM* vm, const char* importer, const char* name) {
  const char* localName = name;
  if (strlen(name) > 1) {
    while (localName[0] == '.' && localName[1] == '/') {
      localName = localName + 2;
    }
  }
  return path_normalize(localName);
}

internal WrenVM* VM_create(ENGINE* engine) {
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = VM_write;
  config.errorFn = VM_error;
  config.bindForeignMethodFn = VM_bind_foreign_method;
  config.bindForeignClassFn = VM_bind_foreign_class;
  config.resolveModuleFn = VM_resolve_module_name;
  config.loadModuleFn = VM_load_module;

  WrenVM* vm = wrenNewVM(&config);
  wrenSetUserData(vm, engine);

  // Set modules

  // StringUtils
  MAP_addFunction(&engine->moduleMap, "stringUtils", "static StringUtils.toLowercase(_)", STRING_UTILS_toLowercase);
  MAP_lockModule(&engine->moduleMap, "stringUtils");

  // DOME
  MAP_addFunction(&engine->moduleMap, "dome", "static Process.f_exit(_)", PROCESS_exit);
  MAP_addFunction(&engine->moduleMap, "dome", "static Process.args", PROCESS_getArguments);
  MAP_addFunction(&engine->moduleMap, "dome", "static Process.errorDialog", PROCESS_getErrorDialog);
  MAP_addFunction(&engine->moduleMap, "dome", "static Process.errorDialog=(_)", PROCESS_setErrorDialog);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.resize(_,_)", WINDOW_resize);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.title=(_)", WINDOW_setTitle);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.vsync=(_)", WINDOW_setVsync);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.lockstep=(_)", WINDOW_setLockStep);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.title", WINDOW_getTitle);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.fullscreen=(_)", WINDOW_setFullscreen);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.fullscreen", WINDOW_getFullscreen);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.width", WINDOW_getWidth);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.height", WINDOW_getHeight);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.fps", WINDOW_getFps);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.f_color=(_)", WINDOW_setColor);
  MAP_addFunction(&engine->moduleMap, "dome", "static Window.f_color", WINDOW_getColor);
  MAP_addFunction(&engine->moduleMap, "dome", "static Version.toString", VERSION_getString);
  MAP_lockModule(&engine->moduleMap, "dome");

  // Canvas
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_pset(_,_,_)", CANVAS_pset);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_pget(_,_)", CANVAS_pget);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_rectfill(_,_,_,_,_)", CANVAS_rectfill);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_cls(_)", CANVAS_cls);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_rect(_,_,_,_,_)", CANVAS_rect);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_line(_,_,_,_,_,_)", CANVAS_line);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_circle(_,_,_,_)", CANVAS_circle);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_circlefill(_,_,_,_)", CANVAS_circle_filled);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_ellipse(_,_,_,_,_)", CANVAS_ellipse);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_ellipsefill(_,_,_,_,_)", CANVAS_ellipsefill);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_print(_,_,_,_)", CANVAS_print);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.offset(_,_)", CANVAS_offset);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.clip(_,_,_,_)", CANVAS_clip);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.f_resize(_,_,_)", CANVAS_resize);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.width", CANVAS_getWidth);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.height", CANVAS_getHeight);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.offsetX", CANVAS_getOffsetX);
  MAP_addFunction(&engine->moduleMap, "graphics", "static Canvas.offsetY", CANVAS_getOffsetY);
  MAP_lockModule(&engine->moduleMap, "graphics");

  // Font
  MAP_addClass(&engine->moduleMap, "font", "FontFile", FONT_allocate, FONT_finalize);
  MAP_addClass(&engine->moduleMap, "font", "RasterizedFont", FONT_RASTER_allocate, FONT_RASTER_finalize);
  MAP_addFunction(&engine->moduleMap, "font", "RasterizedFont.f_print(_,_,_,_)", FONT_RASTER_print);
  MAP_addFunction(&engine->moduleMap, "font", "RasterizedFont.f_getArea(_)", FONT_RASTER_getArea);
  MAP_addFunction(&engine->moduleMap, "font", "RasterizedFont.antialias=(_)", FONT_RASTER_setAntiAlias);
  MAP_lockModule(&engine->moduleMap, "font");

  // Image
  MAP_addClass(&engine->moduleMap, "image", "ImageData", IMAGE_allocate, IMAGE_finalize);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.f_loadFromFile(_)", IMAGE_loadFromFile);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.f_getPNG()", IMAGE_getPNG);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.draw(_,_)", IMAGE_draw);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.width", IMAGE_getWidth);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.height", IMAGE_getHeight);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.f_pget(_,_)", IMAGE_pget);
  MAP_addFunction(&engine->moduleMap, "image", "ImageData.f_pset(_,_,_)", IMAGE_pset);
  MAP_addClass(&engine->moduleMap, "image", "DrawCommand", DRAW_COMMAND_allocate, DRAW_COMMAND_finalize);
  MAP_addFunction(&engine->moduleMap, "image", "DrawCommand.draw(_,_)", DRAW_COMMAND_draw);
  MAP_lockModule(&engine->moduleMap, "image");

  // Audio
  MAP_addClass(&engine->moduleMap, "audio", "SystemChannel", AUDIO_CHANNEL_allocate, AUDIO_CHANNEL_finalize);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.loop=(_)", AUDIO_CHANNEL_setLoop);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.loop", AUDIO_CHANNEL_getLoop);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.pan=(_)", AUDIO_CHANNEL_setPan);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.pan", AUDIO_CHANNEL_getPan);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.volume", AUDIO_CHANNEL_getVolume);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.volume=(_)", AUDIO_CHANNEL_setVolume);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.position=(_)", AUDIO_CHANNEL_setPosition);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.state=(_)", AUDIO_CHANNEL_setState);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.audio=(_)", AUDIO_CHANNEL_setAudio);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.stop()", AUDIO_CHANNEL_stop);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.state", AUDIO_CHANNEL_getState);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.length", AUDIO_CHANNEL_getLength);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.position", AUDIO_CHANNEL_getPosition);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.soundId", AUDIO_CHANNEL_getSoundId);
  MAP_addFunction(&engine->moduleMap, "audio", "SystemChannel.id", AUDIO_CHANNEL_getId);

  // AudioData
  MAP_addClass(&engine->moduleMap, "audio", "AudioData", AUDIO_allocate, AUDIO_finalize);
  MAP_addFunction(&engine->moduleMap, "audio", "AudioData.length", AUDIO_getLength);
  MAP_addFunction(&engine->moduleMap, "audio", "static AudioEngine.f_stopAllChannels()", AUDIO_ENGINE_wrenStopAll);

  MAP_lockModule(&engine->moduleMap, "audio");

  // FileSystem
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.f_load(_,_)", FILESYSTEM_loadAsync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.load(_)", FILESYSTEM_loadSync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.save(_,_)", FILESYSTEM_saveSync);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.listFiles(_)", FILESYSTEM_listFiles);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.listDirectories(_)", FILESYSTEM_listDirectories);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.prefPath(_,_)", FILESYSTEM_getPrefPath);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.basePath()", FILESYSTEM_getBasePath);
  MAP_addFunction(&engine->moduleMap, "io", "static FileSystem.createDirectory(_)", FILESYSTEM_createDirectory);

  // Buffer
  MAP_addClass(&engine->moduleMap, "io", "DataBuffer", DBUFFER_allocate, DBUFFER_finalize);
  MAP_addFunction(&engine->moduleMap, "io", "static DataBuffer.f_capture()", DBUFFER_capture);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.f_data", DBUFFER_getData);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.ready", DBUFFER_getReady);
  MAP_addFunction(&engine->moduleMap, "io", "DataBuffer.f_length", DBUFFER_getLength);

  // AsyncOperation
  MAP_addClass(&engine->moduleMap, "io", "AsyncOperation", ASYNCOP_allocate, ASYNCOP_finalize);
  MAP_addFunction(&engine->moduleMap, "io", "AsyncOperation.result", ASYNCOP_getResult);
  MAP_addFunction(&engine->moduleMap, "io", "AsyncOperation.complete", ASYNCOP_getComplete);
  MAP_lockModule(&engine->moduleMap, "io");

  // Input
  MAP_addClass(&engine->moduleMap, "input", "SystemGamePad", GAMEPAD_allocate, GAMEPAD_finalize);
  MAP_addFunction(&engine->moduleMap, "input", "static Input.f_captureVariables()", INPUT_capture);
  MAP_addFunction(&engine->moduleMap, "input", "static Keyboard.handleText", KEYBOARD_getHandleText);
  MAP_addFunction(&engine->moduleMap, "input", "static Keyboard.handleText=(_)", KEYBOARD_setHandleText);
  MAP_addFunction(&engine->moduleMap, "input", "static Keyboard.textRegion(_,_,_,_)", KEYBOARD_setTextRegion);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.x", MOUSE_getX);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.y", MOUSE_getY);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.scrollX", MOUSE_getScrollX);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.scrollY", MOUSE_getScrollY);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.hidden=(_)", MOUSE_setHidden);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.hidden", MOUSE_getHidden);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.relative=(_)", MOUSE_setRelative);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.relative", MOUSE_getRelative);
  MAP_addFunction(&engine->moduleMap, "input", "static Mouse.setCursor(_)", MOUSE_setCursor);
  MAP_addFunction(&engine->moduleMap, "input", "static SystemGamePad.f_getGamePadIds()", GAMEPAD_getGamePadIds);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.getTrigger(_)", GAMEPAD_getTrigger);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.close()", GAMEPAD_close);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.f_getAnalogStick(_)", GAMEPAD_getAnalogStick);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.rumble(_,_)", GAMEPAD_rumble);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.attached", GAMEPAD_isAttached);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.name", GAMEPAD_getName);
  MAP_addFunction(&engine->moduleMap, "input", "SystemGamePad.id", GAMEPAD_getId);
  MAP_addFunction(&engine->moduleMap, "input", "static Clipboard.content", CLIPBOARD_getContent);
  MAP_addFunction(&engine->moduleMap, "input", "static Clipboard.content=(_)", CLIPBOARD_setContent);
  MAP_lockModule(&engine->moduleMap, "input");

  // Json
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.stream_begin(_)", JSON_STREAM_begin);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.stream_end()", JSON_STREAM_end);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.next", JSON_STREAM_next);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.value", JSON_STREAM_value);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.error_message", JSON_STREAM_error_message);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.lineno", JSON_STREAM_lineno);
  MAP_addFunction(&engine->moduleMap, "json", "JsonStream.pos", JSON_STREAM_pos);
  MAP_addFunction(&engine->moduleMap, "json", "static JsonStream.escapechar(_,_)", JSON_STREAM_escapechar);
  MAP_lockModule(&engine->moduleMap, "json");

  // Platform
  MAP_addFunction(&engine->moduleMap, "platform", "static Platform.time", PLATFORM_getTime);
  MAP_addFunction(&engine->moduleMap, "platform", "static Platform.name", PLATFORM_getName);
  MAP_lockModule(&engine->moduleMap, "platform");

  // Plugin
  MAP_addFunction(&engine->moduleMap, "plugin", "static Plugin.f_load(_)", PLUGIN_load);
  MAP_lockModule(&engine->moduleMap, "plugin");

  // Random
  MAP_addClass(&engine->moduleMap, "random", "Squirrel3", RANDOM_allocate, RANDOM_finalize);
  MAP_addFunction(&engine->moduleMap, "random", "static Squirrel3.noise(_,_)", RANDOM_noise);
  MAP_addFunction(&engine->moduleMap, "random", "Squirrel3.float()", RANDOM_float);

  MAP_addClass(&engine->moduleMap, "random", "Squirrel5", SQUIRREL5_allocate, SQUIRREL5_finalize);
  MAP_addFunction(&engine->moduleMap, "random", "static Squirrel5.noise(_,_)", SQUIRREL5_noise);
  MAP_addFunction(&engine->moduleMap, "random", "Squirrel5.float()", SQUIRREL5_float);
  MAP_lockModule(&engine->moduleMap, "random");

  return vm;
}

internal void VM_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
