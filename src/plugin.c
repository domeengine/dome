internal DOME_Result
ENGINE_registerPlugin(ENGINE* engine, const char* name) {
  void* handle = SDL_LoadObject(name);
  if (handle == NULL) {
    ENGINE_printLog(engine, "%s\n", SDL_GetError());
    return DOME_RESULT_FAILURE;
  }

  PLUGIN_MAP_NODE* head = engine->pluginMap;
  PLUGIN_MAP_NODE* newNode = malloc(sizeof(PLUGIN_MAP_NODE));
  newNode->name = strdup(name);
  newNode->objectHandle = handle;

  newNode->next = head;
  engine->pluginMap = newNode;

  return DOME_RESULT_SUCCESS;
}

internal void
ENGINE_unloadPlugin(ENGINE* engine, const char* name) {
  PLUGIN_MAP_NODE* prev;
  PLUGIN_MAP_NODE* current = engine->pluginMap;
  while (current != NULL) {
    if (STRINGS_EQUAL(current->name, name)) {
      if (prev != NULL) {
        prev->next = current->next;
      } else {
        engine->pluginMap = current->next;
      }
      SDL_UnloadObject(current->objectHandle);
      free(current->name);
      free(current);
      break;
    }
    prev = current;
    current = current->next;
  }
}
