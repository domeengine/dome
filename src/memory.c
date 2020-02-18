typedef struct {
  void* base;
  char* next;
  size_t size;
} MEMORY;

typedef struct {
  size_t size;
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
  memory.size = size;
  //printf("START: %p\n", (void*)memory.next);
}

internal void*
MEMORY_realloc(void* ptr, size_t new_size) {
  //printf("(%zx) ", new_size + sizeof(MEM_HEADER));
  void* result = NULL;
  MEM_HEADER* oldHeader = NULL;
  if (ptr != NULL) {
    oldHeader = (MEM_HEADER*)((char*)ptr - sizeof(MEM_HEADER));
    //printf("%p ", (void*)(&oldHeader->data));
  } else {
    //printf("NULL ");

  }

  if (new_size > 0) {
    result = memory.next;
    MEM_HEADER* header = result;
    if (oldHeader != NULL) {
      memcpy(&header->data, &oldHeader->data, min(new_size, oldHeader->size));
    }
    header->size = new_size;
    result = &header->data;
    memory.next += new_size + sizeof(MEM_HEADER);


    //printf("-> %p\n", (void*)&header->data);
  } else {
    result = NULL;
    //printf("-> NULL\n");
  }
  return result;
}

