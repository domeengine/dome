void INPUT_is_key_down(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  const char* keyName = wrenGetSlotString(vm, 1); 
  SDL_Keycode keycode =  SDL_GetKeyFromName(keyName);
  bool result = ENGINE_getKeyState(engine, keycode).isPressed;
  wrenSetSlotBool(vm, 0, result); 
}

void GRAPHICS_pset(WrenVM* vm) 
{ 
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1)); 
  int16_t y = floor(wrenGetSlotDouble(vm, 2)); 
  uint32_t c = floor(wrenGetSlotDouble(vm, 3)); 
  ENGINE_pset(engine, x,y,c);
}

void GRAPHICS_rectfill(WrenVM* vm) 
{ 
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int16_t x = floor(wrenGetSlotDouble(vm, 1)); 
  int16_t y = floor(wrenGetSlotDouble(vm, 2)); 
  int16_t w = floor(wrenGetSlotDouble(vm, 3)); 
  int16_t h = floor(wrenGetSlotDouble(vm, 4)); 
  uint32_t c = floor(wrenGetSlotDouble(vm, 5)); 
  ENGINE_rectfill(engine, x, y, w, h, c);
}

WrenForeignMethodFn WREN_bind_foreign_method( 
    WrenVM* vm, 
    const char* module, 
    const char* className, 
    bool isStatic, 
    const char* signature) {

  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  ForeignFunctionMap fnMap = engine->fnMap;
  return MAP_get(&fnMap, module, className, signature, isStatic);
}

char* WREN_load_module(WrenVM* vm, const char* name) {
  char* base = "src/engine/";
  char* extension = ".wren";

  char* path;
  path = malloc(strlen(base)+strlen(name)+strlen(extension)+1); /* make space for the new string (should check the return value ...) */
  strcpy(path, base); /* copy name into the new var */
  strcat(path, name); /* add the extension */
  strcat(path, extension); /* add the extension */

  return readEntireFile(path);
}

// Debug output for VM
void WREN_write(WrenVM* vm, const char* text) {
  printf("%s", text);
}

void WREN_error( 
    WrenVM* vm,
    WrenErrorType type, 
    const char* module, 
    int line, 
    const char* message) {
  if (type == WREN_ERROR_COMPILE) {
    printf("%s:%d: %s\n", module, line, message);
  } else if (type == WREN_ERROR_RUNTIME) {
    printf("Runtime error: %s\n", message);
  } else if (type == WREN_ERROR_STACK_TRACE) {
    printf("  %d: %s\n", line, module);
  } 
}

WrenVM* WREN_create(ENGINE* engine) {
  WrenConfiguration config; 
  wrenInitConfiguration(&config);
  config.writeFn = WREN_write; 
  config.errorFn = WREN_error; 
  config.bindForeignMethodFn = WREN_bind_foreign_method; 
  config.loadModuleFn = WREN_load_module; 

  WrenVM* vm = wrenNewVM(&config);
  wrenSetUserData(vm, engine);

  // Set modules
  MAP_add(&engine->fnMap, "graphics", "Graphics", "pset(_,_,_)", true, GRAPHICS_pset);
  MAP_add(&engine->fnMap, "graphics", "Graphics", "rectfill(_,_,_,_,_)", true, GRAPHICS_rectfill);
  MAP_add(&engine->fnMap, "input", "Keyboard", "isKeyDown(_)", true, INPUT_is_key_down);
  
  return vm;
}

void WREN_free(WrenVM* vm) {
  if (vm != NULL) {
    wrenFreeVM(vm);
  }
}
