// Hash table based on the implementation in "Crafting Interpreters"
#define TABLE_MAX_LOAD 0.75
#define NIL_KEY 0

global_variable const GENERIC_CHANNEL TOMBSTONE  = {
  .state = CHANNEL_LAST
};
global_variable const GENERIC_CHANNEL EMPTY_CHANNEL  = {
  .state = CHANNEL_INVALID
};

typedef struct {
  uint32_t key;
  GENERIC_CHANNEL value;
} ENTRY;

typedef struct {
   uint32_t count;
   uint32_t capacity;
  ENTRY* entries;
} TABLE;

void TABLE_init(TABLE* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void TABLE_free(TABLE* table) {
  if (table->entries != NULL) {
    free(table->entries);
  }
  TABLE_init(table);
}

// FNV-1a Hash algorithm, as copied from "Crafting Interpreters"
internal uint32_t
hashData(const void* key, size_t length) {
  uint32_t hash = 2166136261u;
  const char* keyPtr = key;

  for (int i = 0; i < length; i++) {
    hash ^= keyPtr[i];
    hash *= 16777619;
  }

  return hash;
}

#define HASH(key) hashData(&key, sizeof(uintmax_t))

internal ENTRY*
TABLE_findEntry(ENTRY* entries, uint32_t capacity, uintmax_t key) {
  uint32_t index = HASH(key) % capacity;
  ENTRY* tombstone = NULL;
  for (;;) {
    ENTRY* entry = &entries[index];
    if (entry->key == NIL_KEY) {
      if (entry->value.state == CHANNEL_INVALID) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL) {
          tombstone = entry;
        }
      }
    } else if (entry->key == key) {
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

internal void
TABLE_resize(TABLE* table, uint32_t capacity) {
  ENTRY* entries = malloc(sizeof(ENTRY) * capacity);
  for (uint32_t i = 0; i < capacity; i++) {
    entries[i].key = NIL_KEY;
    // dubious
    entries[i].value = EMPTY_CHANNEL;
  }
  table->count = 0;
  for (uint32_t i = 0; i < table->capacity; i++) {
    ENTRY* entry = &table->entries[i];
    if (entry->key == NIL_KEY) {
      continue;
    }
    ENTRY* dest = TABLE_findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }
  free(table->entries);

  table->capacity = capacity;
  table->entries = entries;
}

#define IS_TOMBSTONE(entry) entry->value.state == CHANNEL_LAST
#define IS_EMPTY(entry) entry->value.state == CHANNEL_INVALID

internal bool
TABLE_set(TABLE* table, uintmax_t key, GENERIC_CHANNEL channel) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    uint32_t capacity = table->capacity < 8 ? 8 : table->capacity * 2;
    TABLE_resize(table, capacity);
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry == NULL;

  if (isNewKey && IS_EMPTY(entry)) {
    table->count++;
  }

  entry->key = key;
  entry->value = channel;
  return isNewKey;
}

internal bool
TABLE_get(TABLE* table, uintmax_t key, GENERIC_CHANNEL* channel) {
  if (table->count == 0) {
    return false;
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  if (entry->key == NIL_KEY) {
    return false;
  }
  *channel = entry->value;
  return true;
}

internal bool
TABLE_delete(TABLE* table, uintmax_t key) {
  if (table->count == 0) {
    return false;
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  if (entry->key == NIL_KEY) {
    return false;
  }
  // set a tombstone
  entry->key = NIL_KEY;
  entry->value = TOMBSTONE;
  return true;
}

internal void
TABLE_addAll(TABLE* dest, TABLE* src) {
  for (uint32_t i = 0; i < src->capacity; i++) {
    ENTRY* entry = &src->entries[i];
    if (entry->key != NIL_KEY) {
      TABLE_set(dest, entry->key, entry->value);
    }
  }
}
