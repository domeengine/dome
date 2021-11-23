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

DOME_API_v0 dome_v0 = {
  .registerModule = DOME_registerModuleImpl,
  .registerFn = DOME_registerFnImpl,
  .registerClass = DOME_registerClassImpl,
  .lockModule = DOME_lockModuleImpl,
  .getContext = DOME_getVMContext,
  .getVM = DOME_getVM,
  .log = DOME_printLog
};
