// Hash table based on the implementation in "Crafting Interpreters"
#define TABLE_MAX_LOAD 0.75
#define NIL_KEY 0

#define IS_TOMBSTONE(entry) ((entry)->value.state == CHANNEL_LAST)
#define IS_EMPTY(entry) ((entry)->value.state == CHANNEL_INVALID)

global_variable const CHANNEL TOMBSTONE  = {
  .state = CHANNEL_LAST
};
global_variable const CHANNEL EMPTY_CHANNEL  = {
  .state = CHANNEL_INVALID
};

typedef struct {
  uint32_t next;
  uint32_t found;
  bool done;
  CHANNEL* value;
} TABLE_ITERATOR;

void TABLE_iterInit(TABLE_ITERATOR* iter) {
  iter->next = 0;
  iter->found = 0;
  iter->done = false;
  iter->value = NULL;
}

typedef struct {
  uint32_t key;
  CHANNEL value;
} ENTRY;

typedef struct {
  uint32_t items;
  uint32_t count;
  uint32_t capacity;
  ENTRY* entries;
} TABLE;

void TABLE_init(TABLE* table) {
  table->count = 0;
  table->items = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void TABLE_free(TABLE* table) {
  if (table->entries != NULL) {
    free(table->entries);
  }
  TABLE_init(table);
}

internal bool
TABLE_iterate(TABLE* table, TABLE_ITERATOR* iter) {
  CHANNEL* value = NULL;
  if (iter->found >= table->items) {
    iter->done = true;
  }
  if (!iter->done) {
    for (uint32_t i = iter->next; i < table->capacity; i++) {
      ENTRY* entry = &table->entries[i];
      if (entry->key == NIL_KEY) {
        continue;
      }
      iter->next = i + 1;
      iter->found++;
      value = &entry->value;
      break;
    }
    iter->value = value;
  }
  return !iter->done;
}

/*
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
*/

// Thomas Wang, Integer Hash Functions.
// http://www.concentric.net/~Ttwang/tech/inthash.htm
internal inline  uint32_t
hashBits(uint64_t hash)
{
  // From v8's ComputeLongHash() which in turn cites:
  // Thomas Wang, Integer Hash Functions.
  // http://www.concentric.net/~Ttwang/tech/inthash.htm
  hash = ~hash + (hash << 18);  // hash = (hash << 18) - hash - 1;
  hash = hash ^ (hash >> 31);
  hash = hash * 21;  // hash = (hash + (hash << 2)) + (hash << 4);
  hash = hash ^ (hash >> 11);
  hash = hash + (hash << 6);
  hash = hash ^ (hash >> 22);
  return (uint32_t)(hash & 0x3fffffff);
}

internal ENTRY*
TABLE_findEntry(ENTRY* entries, uint32_t capacity, CHANNEL_ID key) {
  uint32_t startIndex = hashBits(key) % capacity;
  uint32_t index = startIndex;
  ENTRY* tombstone = NULL;
  do {
    ENTRY* entry = &entries[index];
    if (entry->key == NIL_KEY) {
      if (IS_EMPTY(entry)) {
        // Empty entry
        DEBUG_LOG("For %llu, starting in %llu", key, startIndex);
        DEBUG_LOG("empty at %llu", index);
        return tombstone != NULL ? tombstone : entry;
      } else {
        // tombstone
        if (tombstone == NULL) {
          tombstone = entry;
        }
      }
    } else if (entry->key == key) {
      return entry;
    }
    index = (index + 1) % capacity;
  } while (index != startIndex);

  assert(tombstone != NULL);
  return tombstone;
}

internal void
TABLE_resize(TABLE* table, uint32_t capacity) {
  ENTRY* entries = malloc(sizeof(ENTRY) * capacity);
  for (uint32_t i = 0; i < capacity; i++) {
    entries[i].key = NIL_KEY;
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

internal CHANNEL*
TABLE_set(TABLE* table, CHANNEL_ID key, CHANNEL channel) {
  if ((table->count + 1) > table->capacity * TABLE_MAX_LOAD) {
    uint32_t capacity = table->capacity < 8 ? 8 : table->capacity * 2;
    TABLE_resize(table, capacity);
    DEBUG_LOG("capacity: %u", capacity);
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NIL_KEY;

  if (isNewKey && IS_EMPTY(entry)) {
    table->count++;
    table->items++;
  }
  if (isNewKey) {
    entry->key = key;
    entry->value = channel;
  } else {
    assert(false);
  }

  return &(entry->value);
}

internal bool
TABLE_get(TABLE* table, CHANNEL_ID key, CHANNEL** channel) {
  if (table->count == 0) {
    return false;
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  if (entry->key == NIL_KEY) {
    return false;
  }
  *channel = &entry->value;
  return true;
}

internal bool
TABLE_delete(TABLE* table, CHANNEL_ID key) {
  if (table->count == 0) {
    return false;
  }
  ENTRY* entry = TABLE_findEntry(table->entries, table->capacity, key);
  if (entry->key == NIL_KEY) {
    return false;
  }
  DEBUG_LOG("deleting %u", key);
  // set a tombstone
  entry->key = NIL_KEY;
  entry->value = TOMBSTONE;
  table->items--;
  assert(IS_TOMBSTONE(entry));
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
