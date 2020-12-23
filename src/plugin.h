#ifndef PLUGIN_H
#define PLUGIN_H
typedef struct PLUGIN_MAP_NODE_t {
  struct PLUGIN_MAP_NODE_t* prev;
  struct PLUGIN_MAP_NODE_t* next;
  const char* name;
  void* objectHandle;
} PLUGIN_MAP_NODE;

typedef PLUGIN_MAP_NODE* PLUGIN_MAP;

#endif
