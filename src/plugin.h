#ifndef PLUGIN_H
#define PLUGIN_H

typedef DOME_Result (*DOME_Plugin_Init_Hook) (DOME_getAPIFunction apiFn, DOME_Context context);
typedef DOME_Result (*DOME_foreignFn) (DOME_Context context, WrenVM* vm);

typedef struct {
  // Total allocated size
  size_t max;
  // Number in use
  size_t count;
  bool* active;
  const char** name;
  void** objectHandle;
  DOME_Plugin_Hook* preUpdateHook;
  DOME_Plugin_Hook* postUpdateHook;
  DOME_Plugin_Hook* preDrawHook;
  DOME_Plugin_Hook* postDrawHook;
} PLUGIN_COLLECTION;

typedef enum {
  DOME_PLUGIN_HOOK_PRE_UPDATE,
  DOME_PLUGIN_HOOK_POST_UPDATE,
  DOME_PLUGIN_HOOK_PRE_DRAW,
  DOME_PLUGIN_HOOK_POST_DRAW,
  DOME_PLUGIN_HOOK_UNKNOWN
} DOME_PLUGIN_HOOK;


#endif
