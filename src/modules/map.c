#include "modules.inc"

typedef struct FUNCTION_NODE {
  char* signature;
  WrenForeignMethodFn fn;
  struct FUNCTION_NODE* next;
} FUNCTION_NODE;

typedef struct MODULE_NODE {
  const char* name;
  const char* source;
  bool locked;
  DOME_BindClassFn foreignClassFn;
  struct FUNCTION_NODE* functions;
  struct MODULE_NODE* next;
} MODULE_NODE;

typedef struct {
  MODULE_NODE* head;
} MAP;

internal MODULE_NODE* MAP_getModule(MAP* map, const char* name);

internal bool
MAP_addModule(MAP* map, char* name, const char* source) {

  if (MAP_getModule(map, name) != NULL) {
    return false;
  }

  MODULE_NODE* newNode = malloc(sizeof(MODULE_NODE));

  newNode->source = source;
  newNode->name = name;
  newNode->functions = NULL;
  newNode->foreignClassFn = NULL;
  newNode->locked = false;

  newNode->next = map->head;
  map->head = newNode;

  return true;
}

internal MODULE_NODE*
MAP_getModule(MAP* map, const char* name) {
  MODULE_NODE* node = map->head;
  while (node != NULL) {
    if (STRINGS_EQUAL(node->name, name)) {
      return node;
    } else {
      node = node->next;
    }
  }
  return NULL;
}

internal const char*
MAP_getSource(MAP* map, const char* moduleName) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module == NULL) {
    // We don't have the module, but it might be built into Wren (aka Random,Meta)
    return NULL;
  }

  const char* file = strdup(module->source);
  return file;
}

internal bool
MAP_bindForeignClass(MAP* map, const char* moduleName, DOME_BindClassFn fn) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module != NULL) {
    module->foreignClassFn = fn;
    return true;
  }
  return false;
}

internal bool
MAP_addFunction(MAP* map, const char* moduleName, const char* signature, WrenForeignMethodFn fn) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module != NULL && !module->locked) {
    FUNCTION_NODE* node = (FUNCTION_NODE*) malloc(sizeof(FUNCTION_NODE));
    node->signature = signature;
    node->fn = fn;

    node->next = module->functions;
    module->functions = node;
    return true;
  }
  return false;
}

internal void
MAP_lockModule(MAP* map, const char* name) {
  MODULE_NODE* module = MAP_getModule(map, name);
  if (module != NULL) {
    module->locked = true;
  }
}

internal WrenForeignMethodFn
MAP_getFunction(MAP* map, const char* moduleName, const char* signature) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module == NULL) {
    // We don't have the module, but it might be built into Wren (aka Random,Meta)
    return NULL;
  }
  assert(module != NULL);

  FUNCTION_NODE* node = module->functions;
  while (node != NULL) {
    if (STRINGS_EQUAL(signature, node->signature)) {
      return node->fn;
    } else {
      node = node->next;
    }
  }
  return NULL;
}


internal void
MAP_freeFunctions(FUNCTION_NODE* node) {
  FUNCTION_NODE* nextNode;
  while (node != NULL) {
    nextNode = node->next;
    free(node);
    node = nextNode;
  }
}

internal void
MAP_free(MAP* map) {
  MODULE_NODE* module = map->head;
  MODULE_NODE* nextModule;
  while (module != NULL) {
    MAP_freeFunctions(module->functions);
    nextModule = module->next;
    free(module);
    module = nextModule;
  }
}

internal void
MAP_init(MAP* map) {
  map->head = NULL;
#include "modulemap.c.inc"
}

