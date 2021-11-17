internal char*
PLUGIN_COLLECTION_hookName(DOME_PLUGIN_HOOK hook) {
  switch (hook) {
    case DOME_PLUGIN_HOOK_PRE_UPDATE: return "pre-update";
    case DOME_PLUGIN_HOOK_POST_UPDATE: return "post-update";
    case DOME_PLUGIN_HOOK_PRE_DRAW: return "pre-draw";
    case DOME_PLUGIN_HOOK_POST_DRAW: return "post-draw";
    default: return "unknown";
  }
}

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
  PLUGIN_COLLECTION plugins = engine->plugins;
  plugins.max = 0;
  plugins.count = 0;
  assert(plugins.count <= plugins.max);
  plugins.active = NULL;
  plugins.name = NULL;
  plugins.objectHandle = NULL;
  plugins.preUpdateHook = NULL;
  plugins.postUpdateHook = NULL;
  plugins.preDrawHook = NULL;
  plugins.postDrawHook = NULL;
  for (size_t i = 0; i < plugins.max; i++) {
    PLUGIN_COLLECTION_initRecord(&plugins, i);
  }
  engine->plugins = plugins;
}

internal void
PLUGIN_reportHookError(ENGINE* engine, DOME_PLUGIN_HOOK hook, const char* pluginName) {
  ENGINE_printLog(engine, "DOME cannot continue as the following plugin reported a problem:\n");
  ENGINE_printLog(engine, "Plugin: %s - hook: %s\n", pluginName,  PLUGIN_COLLECTION_hookName(hook));
  ENGINE_printLog(engine, "Aborting.\n");
}

#pragma GCC diagnostic push    //Save actual diagnostics state
#pragma GCC diagnostic ignored "-Wpedantic"    //Disable pedantic
internal inline DOME_Plugin_Hook
acquireHook(void* handle, const char* functionName) {
  return SDL_LoadFunction(handle, functionName);
}

internal inline DOME_Plugin_Init_Hook
acquireInitHook(void* handle, const char* functionName) {
  return SDL_LoadFunction(handle, functionName);
}
#pragma GCC diagnostic pop // Restore diagnostics state


internal void
PLUGIN_COLLECTION_free(ENGINE* engine) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  for (size_t i = 0; i < plugins.count; i++) {
    DOME_Plugin_Hook shutdownHook = acquireHook(plugins.objectHandle[i], "PLUGIN_onShutdown");
    if (shutdownHook != NULL) {
      DOME_Result result = shutdownHook(engine);
      if (result != DOME_RESULT_SUCCESS) {
        PLUGIN_reportHookError(engine, DOME_PLUGIN_HOOK_SHUTDOWN, plugins.name[i]);
      }
    }
    plugins.active[i] = false;

    free((void*)plugins.name[i]);
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

  free((void*)plugins.active);
  plugins.active = NULL;

  free((void*)plugins.name);
  plugins.name = NULL;

  free((void*)plugins.objectHandle);
  plugins.objectHandle = NULL;

  engine->plugins = plugins;

  free((void*)plugins.preUpdateHook);
  free((void*)plugins.postUpdateHook);
  free((void*)plugins.preDrawHook);
  free((void*)plugins.postDrawHook);
}

internal DOME_Result
PLUGIN_COLLECTION_runHook(ENGINE* engine, DOME_PLUGIN_HOOK hook) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  bool failure = false;

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
      PLUGIN_reportHookError(engine, hook, plugins.name[i]);
      failure = true;
      break;
    }
  }
  if (failure) {
    return DOME_RESULT_FAILURE;
  }

  return DOME_RESULT_SUCCESS;
}

internal DOME_Result
PLUGIN_COLLECTION_add(ENGINE* engine, const char* name) {
  void* handle = SDL_LoadObject(name);
  if (handle == NULL) {
    bool shouldFree = false;
    const char* path = resolvePath(name, &shouldFree);
    handle = SDL_LoadObject(path);
    if (shouldFree) {
      free((void*)path);
    }
  }

  if (handle == NULL) {
    ENGINE_printLog(engine, "%s\n", SDL_GetError());
    return DOME_RESULT_FAILURE;
  }

  PLUGIN_COLLECTION plugins = engine->plugins;
  size_t next = plugins.count;

  if (next >= plugins.max) {
    #define PLUGIN_FIELD_REALLOC(FIELD, TYPE) \
    do {\
      void* prev = plugins.FIELD; \
      plugins.FIELD = realloc(plugins.FIELD, sizeof(TYPE) * plugins.max); \
      if (plugins.FIELD == NULL) { \
        plugins.FIELD = prev; \
        ENGINE_printLog(engine, "There was a problem allocating memory for plugins\n"); \
        return DOME_RESULT_FAILURE; \
      }\
    } while(false);

    plugins.max = plugins.max == 0 ? 2 : plugins.max * 2;

    PLUGIN_FIELD_REALLOC(active, bool);
    PLUGIN_FIELD_REALLOC(name, char*);
    PLUGIN_FIELD_REALLOC(objectHandle, void*);
    PLUGIN_FIELD_REALLOC(preUpdateHook, void*);
    PLUGIN_FIELD_REALLOC(postUpdateHook, void*);
    PLUGIN_FIELD_REALLOC(preDrawHook, void*);
    PLUGIN_FIELD_REALLOC(postDrawHook, void*);

    if (next == 0) {
      PLUGIN_COLLECTION_initRecord(&plugins, 0);
    }
    for (size_t i = next + 1; i < plugins.max; i++) {
      PLUGIN_COLLECTION_initRecord(&plugins, i);
    }

   #undef PLUGIN_FIELD_REALLOC
  }
  plugins.active[next] = true;
  plugins.name[next] = strdup(name);
  plugins.objectHandle[next] = handle;
  plugins.count++;

  // Acquire hook function pointers
  DOME_Plugin_Hook hook;
  hook = acquireHook(handle, "PLUGIN_preUpdate");
  if (hook != NULL) {
    plugins.preUpdateHook[next] = hook;
  }
  hook = acquireHook(handle, "PLUGIN_postUpdate");
  if (hook != NULL) {
    plugins.postUpdateHook[next] = hook;
  }
  hook = acquireHook(handle, "PLUGIN_preDraw");
  if (hook != NULL) {
    plugins.preDrawHook[next] = hook;
  }
  hook = acquireHook(handle, "PLUGIN_postDraw");
  if (hook != NULL) {
    plugins.postDrawHook[next] = hook;
  }

  engine->plugins = plugins;

  DOME_Plugin_Init_Hook initHook;
  initHook = acquireInitHook(handle, "PLUGIN_onInit");
  if (initHook != NULL) {
    return initHook(DOME_getAPI, engine);
  }

  return DOME_RESULT_SUCCESS;
}


internal DOME_Result
DOME_registerModuleImpl(DOME_Context ctx, const char* name, const char* source) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  if (MAP_addModule(moduleMap, name, source)) {
    return DOME_RESULT_SUCCESS;
  }
  return DOME_RESULT_FAILURE;
}

internal DOME_Result
DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  if (MAP_addFunction(moduleMap, moduleName, signature, (WrenForeignMethodFn)method)) {
    return DOME_RESULT_SUCCESS;
  }

  return DOME_RESULT_FAILURE;
}

internal DOME_Result
DOME_registerClassImpl(DOME_Context ctx, const char* moduleName, const char* className, DOME_ForeignFn allocate, DOME_FinalizerFn finalize) {

  // TODO: handle null allocate ptr
  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  if (MAP_addClass(moduleMap, moduleName, className, (WrenForeignMethodFn)allocate, (WrenFinalizerFn)finalize)) {
    return DOME_RESULT_SUCCESS;
  }

  return DOME_RESULT_FAILURE;
}

internal void
DOME_lockModuleImpl(DOME_Context ctx, const char* moduleName) {
  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);

  MAP_lockModule(moduleMap, moduleName);
}

internal WrenVM*
DOME_getVM(DOME_Context ctx) {
  ENGINE* engine = (ENGINE*)ctx;
  return engine->vm;
}

internal DOME_Context
DOME_getVMContext(WrenVM* vm) {
  return wrenGetUserData(vm);
}
internal void
DOME_printLog(DOME_Context ctx, const char* text, ...) {
  va_list args;
  va_start(args, text);
  ENGINE_printLogVariadic(ctx, text, args);
  va_end(args);
}

WREN_API_v0 wren_v0 = {
  .getUserData = wrenGetUserData,
  .ensureSlots = wrenEnsureSlots,

  .getSlotType = wrenGetSlotType,
  .getSlotCount = wrenGetSlotCount,
  .abortFiber = wrenAbortFiber,

  .setSlotNull = wrenSetSlotNull,
  .setSlotDouble = wrenSetSlotDouble,
  .setSlotString = wrenSetSlotString,
  .setSlotBytes = wrenSetSlotBytes,
  .setSlotBool = wrenSetSlotBool,
  .setSlotNewForeign = wrenSetSlotNewForeign,
  .setSlotNewList = wrenSetSlotNewList,
  .setSlotNewMap = wrenSetSlotNewMap,

  .getSlotBool = wrenGetSlotBool,
  .getSlotDouble = wrenGetSlotDouble,
  .getSlotString = wrenGetSlotString,
  .getSlotBytes = wrenGetSlotBytes,
  .getSlotForeign = wrenGetSlotForeign,

  .getListCount = wrenGetListCount,
  .getListElement = wrenGetListElement,
  .setListElement = wrenSetListElement,
  .insertInList = wrenInsertInList,

  .getMapCount = wrenGetMapCount,
  .getMapContainsKey = wrenGetMapContainsKey,
  .getMapValue = wrenGetMapValue,
  .setMapValue = wrenSetMapValue,
  .removeMapValue = wrenRemoveMapValue,

  .getVariable = wrenGetVariable,
  .getSlotHandle = wrenGetSlotHandle,
  .setSlotHandle = wrenSetSlotHandle,
  .releaseHandle = wrenReleaseHandle,

  .hasVariable = wrenHasVariable,
  .hasModule = wrenHasModule,
  .call = wrenCall,
  .interpret = wrenInterpret,
};

DOME_API_v0 dome_v0 = {
  .registerModule = DOME_registerModuleImpl,
  .registerFn = DOME_registerFnImpl,
  .registerClass = DOME_registerClassImpl,
  .lockModule = DOME_lockModuleImpl,
  .getContext = DOME_getVMContext,
  .getVM = DOME_getVM,
  .log = DOME_printLog
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
  } else if (api == API_GRAPHICS) {
    if (version == 0) {
      return &graphics_v0;
    }
  } else if (api == API_IO) {
    if (version == 0) {
      return &io_v0;
    }
  }

  return NULL;
}
