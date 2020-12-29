#ifndef PLUGIN_H
#define PLUGIN_H
typedef struct PLUGIN_MAP_NODE_t {
  struct PLUGIN_MAP_NODE_t* prev;
  struct PLUGIN_MAP_NODE_t* next;
  const char* name;
  void* objectHandle;
} PLUGIN_MAP_NODE;


typedef DOME_Result (*DOME_Plugin_Init_Hook) (DOME_getAPI apiFn, DOME_Context context);
typedef DOME_Result (*DOME_Plugin_Hook) (DOME_Context context);
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
} PLUGIN_COLLECTION;

typedef enum {
  DOME_PLUGIN_HOOK_PRE_UPDATE,
  DOME_PLUGIN_HOOK_UNKNOWN


} DOME_PLUGIN_HOOK;

typedef PLUGIN_MAP_NODE* PLUGIN_MAP;

#endif
