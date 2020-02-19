#if DOME_SPEED_FAST
#include <sys/mman.h>

struct MEM_HEADER_t;

typedef struct {
  void* base;
  char* next;
  size_t used;
  size_t size;
  struct MEM_HEADER_t* freeList16;
  struct MEM_HEADER_t* freeList32;
  struct MEM_HEADER_t* freeList64;
  struct MEM_HEADER_t* freeList128;
  struct MEM_HEADER_t* freeList256;
  struct MEM_HEADER_t* freeList512;
  struct MEM_HEADER_t* freeListLarge;
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
  memory.freeList16 = NULL;
  memory.freeList32 = NULL;
  memory.freeList64 = NULL;
  memory.freeList128 = NULL;
  memory.freeList256 = NULL;
  memory.freeList512 = NULL;
  memory.freeListLarge = NULL;
  memory.size = size;
  memory.used = 0;
  printf("START: %p\n", (void*)memory.next);
}

internal void*
MEMORY_realloc(void* ptr, size_t newSize) {
  if ((ptr != NULL && (memory.base > ptr || (char*)ptr > ((char*)memory.base + memory.size)))) {
    return realloc(ptr, newSize);
  }
  if (newSize > 0) {
    // printf("%li\n", newSize);
  }

  void* result = NULL;
  MEM_HEADER* oldHeader = NULL;
  if (ptr != NULL) {
    oldHeader = (MEM_HEADER*)((char*)ptr - sizeof(MEM_HEADER));
    memory.used -= oldHeader->size;
    MEM_HEADER* lastFreed = NULL;
    if (oldHeader->size <= 16) {
      lastFreed = memory.freeList16;
      memory.freeList16 = oldHeader;
    } else if (oldHeader->size <= 32) {
      lastFreed = memory.freeList32;
      memory.freeList32 = oldHeader;
    } else if (oldHeader->size <= 64) {
      lastFreed = memory.freeList64;
      memory.freeList64 = oldHeader;
    } else if (oldHeader->size <= 128) {
      lastFreed = memory.freeList128;
      memory.freeList128 = oldHeader;
    } else if (oldHeader->size <= 256) {
      lastFreed = memory.freeList256;
      memory.freeList256 = oldHeader;
    } else if (oldHeader->size <= 512) {
      lastFreed = memory.freeList512;
      memory.freeList512 = oldHeader;
    } else {
      lastFreed = memory.freeListLarge;
      memory.freeListLarge = oldHeader;
    }
    printf("FREED %li\n", oldHeader->size);
    oldHeader->next = lastFreed;
    if (lastFreed != NULL) {
      lastFreed->prev = oldHeader;
    }
  }

  if (newSize > 0) {
    // Check the free list first
    MEM_HEADER* header = NULL;
    if (newSize <= 16) {
      header = memory.freeList16;
    } else if (newSize <= 32) {
      header = memory.freeList32;
    } else if (newSize <= 64) {
      header = memory.freeList64;
    } else if (newSize <= 128) {
      header = memory.freeList128;
    } else if (newSize <= 256) {
      header = memory.freeList256;
    } else if (newSize <= 512) {
      header = memory.freeList512;
    } else {
      header = memory.freeListLarge;
    }

    while (header != NULL) {
      if (header->size >= newSize) {
        MEM_HEADER* next = header->next;
        MEM_HEADER* prev = header->prev;
        if (header->next != NULL) {
          header->next->prev = prev;
        }
        if (header->prev != NULL) {
          header->prev->next = next;
        } else {
          if (newSize <= 16) {
             memory.freeList16 = next;
          } else if (newSize <= 32) {
             memory.freeList32 = next;
          } else if (newSize <= 64) {
             memory.freeList64 = next;
          } else if (newSize <= 128) {
             memory.freeList128 = next;
          } else if (newSize <= 256) {
             memory.freeList256 = next;
          } else if (newSize <= 512) {
             memory.freeList512 = next;
          } else {
             memory.freeListLarge = header->next;
          }
        }
        printf("REUSED %li\n", header->size);
        break;
      }
      header = header->next;
    }

    if (header == NULL) {
      header = (MEM_HEADER*)memory.next;
      header->size = newSize;
      printf("NEW %li\n", header->size);
    }
    header->prev = NULL;
    header->next = NULL;

    if (oldHeader != NULL) {
      memcpy(&(header->data), &(oldHeader->data), min(newSize, oldHeader->size));
    }
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

#define malloc(size) MEMORY_realloc(NULL, size)
#define realloc(ptr, size) MEMORY_realloc(ptr, size)
#define calloc(num, size) MEMORY_calloc(num, size)
#define free(ptr) MEMORY_realloc(ptr, 0)

#endif
