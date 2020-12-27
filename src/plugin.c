internal DOME_Result
PLUGIN_nullHook(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

internal void
PLUGIN_COLLECTION_initRecord(PLUGIN_COLLECTION* plugins, size_t index) {
  plugins->active[index] = false;
  plugins->objectHandle[index] = NULL;
  plugins->name[index] = NULL;
  plugins->preUpdateHook[index] = PLUGIN_nullHook;
}

internal void
PLUGIN_COLLECTION_init(ENGINE* engine) {
  // TODO: handle allocation failures
  PLUGIN_COLLECTION plugins = engine->plugins;
  plugins.max = 1;
  plugins.count = 0;
  assert(plugins.count <= plugins.max);
  plugins.active = calloc(plugins.max, sizeof(bool));
  plugins.name = calloc(plugins.max, sizeof(char*));
  plugins.objectHandle = calloc(plugins.max, sizeof(void*));
  plugins.preUpdateHook = calloc(plugins.max, sizeof(DOME_Plugin_Hook));
  for (size_t i = 0; i < plugins.max; i++) {
    PLUGIN_COLLECTION_initRecord(&plugins, i);
  }
  engine->plugins = plugins;
}

internal void
PLUGIN_COLLECTION_free(ENGINE* engine) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  for (size_t i = 0; i < plugins.count; i++) {
    // TODO Call the unload here
    DOME_Plugin_Hook shutdownHook;
    shutdownHook = (DOME_Plugin_Hook)SDL_LoadFunction(plugins.objectHandle[i], "DOME_hookOnShutdown");
    if (shutdownHook != NULL) {
      // TODO: provide context object
      shutdownHook(engine);
      // TODO: Log failure
    }
    plugins.active[i] = false;

    free(plugins.name[i]);
    plugins.name[i] = NULL;

    SDL_UnloadObject(plugins.objectHandle[i]);
    plugins.objectHandle[i] = NULL;

    plugins.preUpdateHook[i] = NULL;
  }

  plugins.max = 0;
  plugins.count = 0;

  free(plugins.active);
  plugins.active = NULL;

  free(plugins.name);
  plugins.name = NULL;

  free(plugins.objectHandle);
  plugins.objectHandle = NULL;

  engine->plugins = plugins;
}

internal DOME_Result
PLUGIN_COLLECTION_runHook(ENGINE* engine, DOME_PLUGIN_HOOK hook) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  size_t failures = 0;

  for (size_t i = 0; i < plugins.count; i++) {
    assert(plugins.active[i]);
    DOME_Result result = DOME_RESULT_SUCCESS;
    switch (hook) {
      case DOME_PLUGIN_HOOK_PRE_UPDATE:
        { result = plugins.preUpdateHook[i](engine); } break;
      default: break;
    }
    if (result != DOME_RESULT_SUCCESS) {
      failures++;
      // Do something about errors?
    }
  }

  return DOME_RESULT_SUCCESS;
}

internal DOME_Result
PLUGIN_COLLECTION_add(ENGINE* engine, const char* name) {
  void* handle = SDL_LoadObject(name);
  if (handle == NULL) {
    ENGINE_printLog(engine, "%s\n", SDL_GetError());
    return DOME_RESULT_FAILURE;
  }

  PLUGIN_COLLECTION plugins = engine->plugins;
  size_t next = plugins.count;
  printf("Next is %zu, max is %zu\n", next, plugins.max);

  if (next >= plugins.max) {
    plugins.max *= 2;
    printf("Increasing max to %zu\n", plugins.max);

    plugins.active = realloc(plugins.active, sizeof(bool) * plugins.max);
    plugins.name = realloc(plugins.name, sizeof(char*) * plugins.max);
    plugins.objectHandle = realloc(plugins.objectHandle, sizeof(void*) * plugins.max);
    plugins.preUpdateHook = realloc(plugins.preUpdateHook, sizeof(void*) * plugins.max);
    for (size_t i = next + 1; i < plugins.max; i++) {
      PLUGIN_COLLECTION_initRecord(&plugins, i);
    }
    // TODO handle allocation failure
  }
  plugins.active[next] = true;
  plugins.name[next] = strdup(name);
  plugins.objectHandle[next] = handle;
  plugins.count++;

  // Acquire hook function pointers
  DOME_Plugin_Hook hook;
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "DOME_hookOnPreUpdate");
  if (hook != NULL) {
    plugins.preUpdateHook[next] = hook;
  }

  engine->plugins = plugins;

  DOME_Plugin_Hook initHook;
  initHook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "DOME_hookOnInit");
  if (initHook != NULL) {
    // TODO: provide context object
    return initHook(engine);
  }

  return DOME_RESULT_SUCCESS;
}

DOME_EXPORTED DOME_Result
DOME_registerModule(DOME_Context ctx, const char* name, const char* source) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addModule(moduleMap, name, source);

  return DOME_RESULT_SUCCESS;
}

DOME_EXPORTED DOME_Result
DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addFunction(moduleMap, moduleName, signature, (WrenForeignMethodFn)method);

  return DOME_RESULT_SUCCESS;
}

DOME_EXPORTED DOME_Context
DOME_getContext(void* vm) {
  return wrenGetUserData(vm);
}

DOME_EXPORTED void
DOME_setSlot(void* vm, size_t slot, DOME_SLOT_TYPE type, ...) {
  va_list argp;
  va_start(argp, type);

  double number;
  switch (type) {
    case DOME_SLOT_TYPE_NULL:
      wrenSetSlotNull(vm, slot); break;
    case DOME_SLOT_TYPE_NUMBER:
      number = va_arg(argp, double);
      wrenSetSlotDouble(vm, slot, number);
      break;
    default:
      VM_ABORT(vm, "Unhandled plugin return type"); break;
  }
  va_end(argp);
}
