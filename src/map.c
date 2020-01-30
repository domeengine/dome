/*
  Really dumb string->function map for providing the WrenVM with module/class/method functions.
  This only runs once at the start of execution so we aren't relying on it to be performant.
 */
typedef struct ForeignFunctionMapNode {
  char* module;
  char* signature;
  WrenForeignMethodFn fn;
  struct ForeignFunctionMapNode* next;
} ForeignFunctionMapNode;

typedef struct {
  ForeignFunctionMapNode* head;
} ForeignFunctionMap;

internal void MAP_add(ForeignFunctionMap* map, char* module, char* signature, WrenForeignMethodFn fn) {
  ForeignFunctionMapNode* node = (ForeignFunctionMapNode*) malloc(sizeof(ForeignFunctionMapNode));
  node->module = module;
  node->signature = signature;
  node->fn = fn;
  node->next = map->head;
  map->head = node;
}

internal WrenForeignMethodFn MAP_get(ForeignFunctionMap* map, const char* module, const char* signature) {
  ForeignFunctionMapNode* node = map->head;
  while (node != NULL) {
    if (STRINGS_EQUAL(module, node->module) &&
        STRINGS_EQUAL(signature, node->signature)) {
      return node->fn;
    } else {
      node = node->next;
    }
  }
  return NULL;
}

internal void MAP_free(ForeignFunctionMap* map) {
  ForeignFunctionMapNode* node = map->head;
  ForeignFunctionMapNode* nextNode;
  while (node != NULL) {
    nextNode = node->next;
    free(node);
    node = nextNode;
  }
}
