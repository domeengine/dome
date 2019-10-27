#include "dome.wren.inc"
#include "audio.wren.inc"
#include "graphics.wren.inc"
#include "point.wren.inc"
#include "input.wren.inc"
#include "io.wren.inc"


typedef struct ModuleMapNode_t {
  const char* name;
  const char* module;
  struct ModuleMapNode_t* next;
} ModuleMapNode;

typedef struct {
  ModuleMapNode* head;
} ModuleMap;


internal void
ModuleMap_add(ModuleMap* map, char* name, const char* module) {
  ModuleMapNode* newNode = malloc(sizeof(ModuleMapNode));

  newNode->module = module;
  newNode->name = name;

  newNode->next = map->head;
  map->head = newNode;
}

internal void
ModuleMap_free(ModuleMap* map) {
  ModuleMapNode* node = map->head;
  ModuleMapNode* nextNode;
  while (node != NULL) {
    nextNode = node->next;
    free(node);
    node = nextNode;
  }
}

internal const char*
ModuleMap_get(ModuleMap* map, const char* name) {
  ModuleMapNode* node = map->head;
  while (node != NULL) {
    if (strcmp(node->name, name) == 0) {
      size_t sourceLen = strlen(node->module);
      char* file = calloc(sourceLen + 1, sizeof(char));
      strncpy(file, node->module, sourceLen);
      file[sourceLen] = '\0';
      return file;
    } else {
      node = node->next;
    }
  }
  return NULL;
}

internal void
ModuleMap_init(ModuleMap* map) {
  map->head = NULL;
  ModuleMap_add(map, "dome", domeModule);
  ModuleMap_add(map, "input", inputModule);
  ModuleMap_add(map, "io", ioModule);
  ModuleMap_add(map, "graphics", graphicsModule);
  ModuleMap_add(map, "audio", audioModule);
  ModuleMap_add(map, "point", pointModule);
}
