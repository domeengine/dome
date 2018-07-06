/*
  Really dumb string->function map for providing the WrenVM with module/class/method functions.
  This only runs once at the start of execution so we aren't relying on it to be performant.
 */
typedef struct ForeignFunctionMapNode {
  char* module;
  char* className;
  char* signature;
  bool isStatic; 
  WrenForeignMethodFn fn; 
  struct ForeignFunctionMapNode* next;
} ForeignFunctionMapNode; 

typedef struct {
  ForeignFunctionMapNode* head;
} ForeignFunctionMap; 

internal void MAP_add(ForeignFunctionMap* map, char* module, char* className, char* signature, bool isStatic, WrenForeignMethodFn fn) {
  ForeignFunctionMapNode* node = (ForeignFunctionMapNode*) malloc(sizeof(ForeignFunctionMapNode));
  node->module = module;
  node->className = className;
  node->signature = signature; 
  node->isStatic = isStatic;
  node->fn = fn;
  node->next = map->head;
  map->head = node;
}

internal WrenForeignMethodFn MAP_get(ForeignFunctionMap* map, const char* module, const char* className, const char* signature, bool isStatic) {
  ForeignFunctionMapNode* node = map->head;
  while (node != NULL) {
    if (strcmp(module, node->module) == 0 &&
        strcmp(className, node->className) == 0 &&
        strcmp(signature, node->signature) == 0 &&
        isStatic == node->isStatic) {
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
