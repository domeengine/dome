#include "modules.inc"

typedef struct CLASS_NODE {
  const char* name;
  WrenForeignClassMethods methods;
  struct CLASS_NODE* next;
} CLASS_NODE;

typedef struct FUNCTION_NODE {
  const char* signature;
  WrenForeignMethodFn fn;
  struct FUNCTION_NODE* next;
} FUNCTION_NODE;

typedef struct MODULE_NODE {
  const char* name;
  const char* source;
  bool locked;
  struct CLASS_NODE* classes;
  struct FUNCTION_NODE* functions;
  struct MODULE_NODE* next;
} MODULE_NODE;

typedef struct {
  MODULE_NODE* head;
} MAP;

internal MODULE_NODE* MAP_getModule(MAP* map, const char* name);

internal bool
MAP_addModule(MAP* map, const char* name, const char* source) {

  if (MAP_getModule(map, name) != NULL) {
    return false;
  }

  MODULE_NODE* newNode = malloc(sizeof(MODULE_NODE));

  newNode->source = strdup(source);
  newNode->name = strdup(name);
  newNode->functions = NULL;
  newNode->classes = NULL;
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
MAP_addFunction(MAP* map, const char* moduleName, const char* signature, WrenForeignMethodFn fn) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  // TODO: Check for duplicate?
  if (module != NULL && !module->locked) {
    FUNCTION_NODE* node = (FUNCTION_NODE*) malloc(sizeof(FUNCTION_NODE));
    node->signature = strdup(signature);
    node->fn = fn;

    node->next = module->functions;
    module->functions = node;
    return true;
  }
  return false;
}

internal bool
MAP_addClass(MAP* map, const char* moduleName, const char* className, WrenForeignMethodFn allocate, WrenFinalizerFn finalize) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  // TODO: Check for duplicate?
  if (module != NULL && !module->locked) {
    CLASS_NODE* node = (CLASS_NODE*) malloc(sizeof(CLASS_NODE));
    node->name = strdup(className);
    node->methods.allocate = allocate;
    node->methods.finalize = finalize;

    node->next = module->classes;
    module->classes = node;
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

internal bool
MAP_getClassMethods(MAP* map, const char* moduleName, const char* className, WrenForeignClassMethods* methods) {
  MODULE_NODE* module = MAP_getModule(map, moduleName);
  if (module == NULL) {
    // We don't have the module, but it might be built into Wren (aka Random,Meta)
    return false;
  }
  assert(module != NULL);

  CLASS_NODE* node = module->classes;
  while (node != NULL) {
    if (STRINGS_EQUAL(className, node->name)) {
      *methods = node->methods;
      return true;
    } else {
      node = node->next;
    }
  }
  return false;
}


internal void
MAP_freeFunctions(FUNCTION_NODE* node) {
  FUNCTION_NODE* nextNode;
  while (node != NULL) {
    nextNode = node->next;
    free((char*) node->signature);
    free(node);
    node = nextNode;
  }
}
internal void
MAP_freeClasses(CLASS_NODE* node) {
  CLASS_NODE* nextNode;
  while (node != NULL) {
    nextNode = node->next;
    free((char*) node->name);
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
    MAP_freeClasses(module->classes);
    nextModule = module->next;
    free((char*) module->name);
    free((char*) module->source);
    free(module);
    module = nextModule;
  }
}

internal void
MAP_init(MAP* map) {
  map->head = NULL;
#include "modulemap.c.inc"
}

