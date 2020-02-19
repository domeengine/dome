#if DOME_SPEED_FAST
#include <sys/mman.h>

struct MEM_HEADER_t;

typedef struct {
  void* base;
  char* next;
  size_t used;
  size_t size;
  struct MEM_HEADER_t* freeList;
} MEMORY;

typedef struct MEM_HEADER_t {
  size_t size;
  struct MEM_HEADER_t* next;
  struct MEM_HEADER_t* prev;
  char data[];
} MEM_HEADER;

global_variable MEMORY memory;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

internal void
MEMORY_init(size_t size) {
  memory.base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (memory.base == MAP_FAILED) {
    abort();
  }
  memory.next = memory.base;
  memory.freeList = NULL;
  memory.size = size;
  memory.used = 0;
  printf("START: %p\n", (void*)memory.next);
}

internal void*
MEMORY_realloc(void* ptr, size_t newSize) {
  if (ptr != NULL && (memory.base > ptr || (char*)ptr > ((char*)memory.base + memory.size))) {
    return realloc(ptr, newSize);
  }

  void* result = NULL;
  MEM_HEADER* oldHeader = NULL;
  if (ptr != NULL) {
    oldHeader = (MEM_HEADER*)((char*)ptr - sizeof(MEM_HEADER));
  }

  if (newSize > 0) {
    // Check the free list first
    MEM_HEADER* header = memory.freeList;
    if (header == NULL) {
      header = (MEM_HEADER*)memory.next;
    }
    if (oldHeader != NULL) {
      memcpy(&(header->data), &(oldHeader->data), min(newSize, oldHeader->size));
    }
    header->size = newSize;
//     header->prev = NULL;
//     header->next = NULL;
    result = &(header->data);
    memory.next += newSize + sizeof(MEM_HEADER);
    memory.used += newSize;
  } else {
    result = NULL;
  }
  return result;
}

void* MEMORY_calloc(size_t num, size_t size) {
  void* result = MEMORY_realloc(NULL, num * size);
  memset(result, 0, num * size);
  return result;
}

#endif
