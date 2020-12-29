#include "modules.inc"


typedef struct FUNCTION_NODE {
  char* signature;
  WrenForeignMethodFn fn;
  struct FUNCTION_NODE* next;
} FUNCTION_NODE;

typedef struct MODULE_NODE {
  const char* name;
  const char* source;
  DOME_BindClassFn foreignClassFn;
  struct FUNCTION_NODE* functions;
  struct MODULE_NODE* next;
} MODULE_NODE;

typedef struct {
  MODULE_NODE* head;
} MAP;


internal void
MAP_addModule(MAP* map, char* name, const char* source) {
  MODULE_NODE* newNode = malloc(sizeof(MODULE_NODE));

  newNode->source = source;
  newNode->name = name;
  newNode->functions = NULL;
  newNode->foreignClassFn = NULL;

  newNode->next = map->head;
  map->head = newNode;
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

  size_t sourceLen = strlen(module->source);
  char* file = calloc(sourceLen + 1, sizeof(char));
  strcpy(file, module->source);
  file[sourceLen] = '\0';
  return file;
}

internal DOME_Result
MAP_bindForeignClass(MAP* map, char* moduleName, DOME_BindClassFn fn) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module != NULL) {
    module->foreignClassFn = fn;
    return DOME_RESULT_SUCCESS;
  } else {
    return DOME_RESULT_FAILURE;
  }
}

internal void
MAP_addFunction(MAP* map, char* moduleName, char* signature, WrenForeignMethodFn fn) {
  FUNCTION_NODE* node = (FUNCTION_NODE*) malloc(sizeof(FUNCTION_NODE));
  node->signature = signature;
  node->fn = fn;

  MODULE_NODE* module = MAP_getModule(map, moduleName);
  assert(module != NULL);

  node->next = module->functions;
  module->functions = node;
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

