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
  plugins->postUpdateHook[index] = PLUGIN_nullHook;
  plugins->preDrawHook[index] = PLUGIN_nullHook;
  plugins->postDrawHook[index] = PLUGIN_nullHook;
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
  plugins.postUpdateHook = calloc(plugins.max, sizeof(DOME_Plugin_Hook));
  plugins.preDrawHook = calloc(plugins.max, sizeof(DOME_Plugin_Hook));
  plugins.postDrawHook = calloc(plugins.max, sizeof(DOME_Plugin_Hook));
  for (size_t i = 0; i < plugins.max; i++) {
    PLUGIN_COLLECTION_initRecord(&plugins, i);
  }
  engine->plugins = plugins;
}

internal void
PLUGIN_COLLECTION_free(ENGINE* engine) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  for (size_t i = 0; i < plugins.count; i++) {
    DOME_Plugin_Hook shutdownHook;
    shutdownHook = (DOME_Plugin_Hook)SDL_LoadFunction(plugins.objectHandle[i], "PLUGIN_onShutdown");
    if (shutdownHook != NULL) {
      shutdownHook(engine);
      // TODO: Log failure
    }
    plugins.active[i] = false;

    free(plugins.name[i]);
    plugins.name[i] = NULL;

    SDL_UnloadObject(plugins.objectHandle[i]);
    plugins.objectHandle[i] = NULL;

    plugins.preUpdateHook[i] = NULL;
    plugins.postUpdateHook[i] = NULL;
    plugins.preDrawHook[i] = NULL;
    plugins.postDrawHook[i] = NULL;
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
      case DOME_PLUGIN_HOOK_POST_UPDATE:
        { result = plugins.postUpdateHook[i](engine); } break;
      case DOME_PLUGIN_HOOK_PRE_DRAW:
        { result = plugins.preDrawHook[i](engine); } break;
      case DOME_PLUGIN_HOOK_POST_DRAW:
        { result = plugins.postDrawHook[i](engine); } break;
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
    plugins.postUpdateHook = realloc(plugins.postUpdateHook, sizeof(void*) * plugins.max);
    plugins.preDrawHook = realloc(plugins.preDrawHook, sizeof(void*) * plugins.max);
    plugins.postDrawHook = realloc(plugins.postDrawHook, sizeof(void*) * plugins.max);
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
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "PLUGIN_preUpdate");
  if (hook != NULL) {
    plugins.preUpdateHook[next] = hook;
  }
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "PLUGIN_postUpdate");
  if (hook != NULL) {
    plugins.postUpdateHook[next] = hook;
  }
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "PLUGIN_preDraw");
  if (hook != NULL) {
    plugins.preDrawHook[next] = hook;
  }
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "PLUGIN_postDraw");
  if (hook != NULL) {
    plugins.postDrawHook[next] = hook;
  }

  engine->plugins = plugins;

  DOME_Plugin_Init_Hook initHook;
  initHook = (DOME_Plugin_Init_Hook)SDL_LoadFunction(handle, "PLUGIN_onInit");
  if (initHook != NULL) {
    return initHook(DOME_getAPI, engine);
  }

  return DOME_RESULT_SUCCESS;
}

internal DOME_Result
DOME_registerModuleImpl(DOME_Context ctx, const char* name, const char* source) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addModule(moduleMap, name, source);

  return DOME_RESULT_SUCCESS;
}

internal DOME_Result
DOME_registerBindFnImpl(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn) {
  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  return MAP_bindForeignClass(moduleMap, moduleName, fn);
}

internal DOME_Result
DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addFunction(moduleMap, moduleName, signature, (WrenForeignMethodFn)method);

  return DOME_RESULT_SUCCESS;
}

WREN_API_v0 wren_v0 = {
  .getUserData = wrenGetUserData,

  .setSlotNull = wrenSetSlotNull,
  .setSlotDouble = wrenSetSlotDouble,
  .setSlotString = wrenSetSlotString,
  .setSlotBytes = wrenSetSlotBytes,
  .setSlotBool = wrenSetSlotBool,
  .setSlotNewForeign = wrenSetSlotNewForeign,

  .getSlotBool = wrenGetSlotBool,
  .getSlotDouble = wrenGetSlotDouble,
  .getSlotString = wrenGetSlotString,
  .getSlotBytes = wrenGetSlotBytes,
  .getSlotForeign = wrenGetSlotForeign,

  .abortFiber = wrenAbortFiber
};

DOME_API_v0 dome_v0 = {
  .registerModule = DOME_registerModuleImpl,
  .registerFn = DOME_registerFnImpl,
  .registerBindFn = DOME_registerBindFnImpl,
};


external void*
DOME_getAPI(API_TYPE api, int version) {
  if (api == API_DOME) {
    if (version == 0) {
      return &dome_v0;
    }
  } else if (api == API_WREN) {
    if (version == 0) {
      return &wren_v0;
    }
  } else if (api == API_AUDIO) {
    if (version == 0) {
      return &audio_v0;
    }
  }

  return NULL;
}
