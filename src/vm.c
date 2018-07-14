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

  if (strcmp(module, "audio") == 0) {
    if (strcmp(className, "AudioData") == 0) {
      methods.allocate = AUDIO_allocate;
      methods.finalize = AUDIO_finalize;
    } else if (strcmp(className, "AudioEngineImpl") == 0) {
      methods.allocate = AUDIO_ENGINE_allocate;
      methods.finalize = AUDIO_ENGINE_finalize;
    }
  }

  if (strcmp(className, "Point") == 0) {
    methods.allocate = POINT_allocate;
    methods.finalize = POINT_finalize;
  }
  return methods;
}


internal void INPUT_is_key_down(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* keyName = wrenGetSlotString(vm, 1);
  SDL_Keycode keycode =  SDL_GetKeyFromName(keyName);
  bool result = ENGINE_getKeyState(engine, keycode).isPressed;
  wrenSetSlotBool(vm, 0, result);
}

internal void CANVAS_print(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  char* text = (char*)wrenGetSlotString(vm, 1);
  int16_t x = floor(wrenGetSlotDouble(vm, 2));
  int16_t y = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));

  ENGINE_print(engine, text, x, y, c);
}

internal void CANVAS_pset(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  uint32_t c = floor(wrenGetSlotDouble(vm, 3));
  ENGINE_pset(engine, x,y,c);
}

internal void CANVAS_circle_filled(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t r = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));
  ENGINE_circle_filled(engine, x, y, r, c);
}

internal void CANVAS_circle(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t r = floor(wrenGetSlotDouble(vm, 3));
  uint32_t c = floor(wrenGetSlotDouble(vm, 4));
  ENGINE_circle(engine, x, y, r, c);
}
internal void CANVAS_line(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x1 = floor(wrenGetSlotDouble(vm, 1));
  int16_t y1 = floor(wrenGetSlotDouble(vm, 2));
  int16_t x2 = floor(wrenGetSlotDouble(vm, 3));
  int16_t y2 = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_line(engine, x1, y1, x2, y2, c);
}

internal void CANVAS_rect(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t w = floor(wrenGetSlotDouble(vm, 3));
  int16_t h = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_rect(engine, x, y, w, h, c);
}
internal void CANVAS_rectfill(WrenVM* vm)
{
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1));
  int16_t y = floor(wrenGetSlotDouble(vm, 2));
  int16_t w = floor(wrenGetSlotDouble(vm, 3));
  int16_t h = floor(wrenGetSlotDouble(vm, 4));
  uint32_t c = floor(wrenGetSlotDouble(vm, 5));
  ENGINE_rectfill(engine, x, y, w, h, c);
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
  char* base = "src/engine/";
  char* extension = ".wren";

  char* path;
  path = malloc(strlen(base)+strlen(name)+strlen(extension)+1); /* make space for the new string (should check the return value ...) */
  strcpy(path, base); /* copy name into the new var */
  strcat(path, name); /* add the extension */
  strcat(path, extension); /* add the extension */

  char* file = readEntireFile(path);
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

  // Canvas
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_pset(_,_,_)", true, CANVAS_pset);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_rectfill(_,_,_,_,_)", true, CANVAS_rectfill);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_rect(_,_,_,_,_)", true, CANVAS_rect);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_line(_,_,_,_,_)", true, CANVAS_line);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_circle(_,_,_,_)", true, CANVAS_circle);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_circlefill(_,_,_,_)", true, CANVAS_circle_filled);
  MAP_add(&engine->fnMap, "graphics", "Canvas", "f_print(_,_,_,_)", true, CANVAS_print);
  MAP_add(&engine->fnMap, "graphics", "ImageData", "draw(_,_)", false, IMAGE_draw);

  // Audio
  MAP_add(&engine->fnMap, "audio", "AudioData", "unload()", false, AUDIO_unload);
  MAP_add(&engine->fnMap, "audio", "AudioEngineImpl", "update()", false, AUDIO_ENGINE_update);


  // Input
  MAP_add(&engine->fnMap, "input", "Keyboard", "isKeyDown(_)", true, INPUT_is_key_down);

  // Point
  MAP_add(&engine->fnMap, "graphics", "Point", "x", false, POINT_getX);
  MAP_add(&engine->fnMap, "graphics", "Point", "y", false, POINT_getY);
  return vm;
}

internal void VM_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
