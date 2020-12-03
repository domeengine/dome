/*
  ABC_fifo.h - v0.0.12 - Public Domain
  Author: Aviv Beeri, 2018

  How To Use:


  #define ABC_FIFO_IMPL
  #include "ABC_fifo.h"

  Version History:

  v0.0.12 - Moved the SDL warning so ABC_fifo can be compiled in a seperate translation unit.
  v0.0.11 - When closing, we actually make sure all the threads can wake up
            waiting for them all.
  v0.0.10 - We block when closing the FIFO til all threads finish
  v0.0.9 - Initial Release, be careful

  License:

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/


#ifndef ABC_FIFO_H
#define ABC_FIFO_H

// Initial configuration
#ifndef ABC_FIFO_POOL_SIZE
  #define ABC_FIFO_POOL_SIZE 4
#endif
#ifndef ABC_FIFO_SIZE
  #define ABC_FIFO_SIZE 256
#endif

typedef uint8_t ABC_TASK_TYPE;

typedef struct {
  int resultCode;
  ABC_TASK_TYPE type;
  void* data;
  void* returnData;
} ABC_TASK;

typedef int (*ABC_FIFO_TaskHandler)(ABC_TASK* task);
typedef struct {
  SDL_sem* semaphore;
  SDL_atomic_t readEntry;
  SDL_atomic_t completionCount;
  volatile uint16_t writeEntry;

  volatile bool shutdown;
  ABC_FIFO_TaskHandler taskHandler;
  SDL_Thread* threads[ABC_FIFO_POOL_SIZE];

  ABC_TASK tasks[ABC_FIFO_SIZE];
} ABC_FIFO;

// Public API:
void ABC_FIFO_create(ABC_FIFO* queue);
void ABC_FIFO_pushTask(ABC_FIFO* queue, ABC_TASK task);
bool ABC_FIFO_isFull(ABC_FIFO* queue);
bool ABC_FIFO_isEmpty(ABC_FIFO* queue);
void ABC_FIFO_waitForEmptyQueue(ABC_FIFO* queue);
void ABC_FIFO_close(ABC_FIFO* queue);

#endif // ABC_FIFO_ABC_FIFO_H




#ifdef SDL_INIT_EVERYTHING

#ifdef ABC_FIFO_IMPL
#define ABC_FIFO_IMPL


#ifndef SDL_INIT_EVERYTHING
#error "ABC_FIFO depends on SDL, which could not be detected."
#define SDL_sem void
#define SDL_atomic_t int
#define SDL_Thread int
#endif




int16_t mask(int16_t v) {
  return v & (ABC_FIFO_SIZE - 1);
}




int ABC_FIFO_executeTask(void* data);

// Initialise the queue passed in
void ABC_FIFO_create(ABC_FIFO* queue) {
  memset(queue, 0, sizeof(ABC_FIFO));
  queue->semaphore = SDL_CreateSemaphore(0);

  for (int i = 0; i < ABC_FIFO_POOL_SIZE; ++i) {
    queue->threads[i] = SDL_CreateThread(ABC_FIFO_executeTask, NULL, queue);
  }
}

// PRE: Only the main thread pushes tasks
// Blocks if the queue is full.
void ABC_FIFO_pushTask(ABC_FIFO* queue, ABC_TASK task) {
  while(ABC_FIFO_isFull(queue)) { };

  queue->tasks[mask(queue->writeEntry)] = task;
  SDL_CompilerBarrier();
  ++queue->writeEntry;

  if (SDL_SemValue(queue->semaphore) < ABC_FIFO_POOL_SIZE) {
    SDL_SemPost(queue->semaphore);
  }
}

int ABC_FIFO_executeTask(void* data) {
  ABC_FIFO* queue = (ABC_FIFO*)data;
  while (!queue->shutdown) {
   int16_t originalReadEntry = SDL_AtomicGet(&queue->readEntry);
   if (mask(originalReadEntry) != mask(queue->writeEntry)) {
     if (SDL_AtomicCAS(&queue->readEntry, originalReadEntry, originalReadEntry + 1)) {
       SDL_CompilerBarrier();
       ABC_TASK task = queue->tasks[mask(originalReadEntry)];

       queue->taskHandler(&task);
       SDL_AtomicAdd(&queue->completionCount, 1); // Post-increment
     }
    } else {
      SDL_SemWait(queue->semaphore);
    }
  }
  return 0;
}

// Shutdown pool and block til all threads close
void ABC_FIFO_close(ABC_FIFO* queue) {
  queue->shutdown = true;
  // Tidy up our resources
  for (int i = 0; i < ABC_FIFO_POOL_SIZE; ++i) {
    SDL_SemPost(queue->semaphore);
  }
  for (int i = 0; i < ABC_FIFO_POOL_SIZE; ++i) {
    SDL_WaitThread(queue->threads[i], NULL);
  }

  // Destroy queue
  SDL_DestroySemaphore(queue->semaphore);
}

bool ABC_FIFO_isFull(ABC_FIFO* queue) {
  return mask(queue->writeEntry + 1) == mask(SDL_AtomicGet(&queue->readEntry));
}

bool ABC_FIFO_isEmpty(ABC_FIFO* queue) {
  return mask(queue->writeEntry) == mask(SDL_AtomicGet(&queue->readEntry));
}

void ABC_FIFO_waitForEmptyQueue(ABC_FIFO* queue) {
  while (!ABC_FIFO_isEmpty(queue));
}

#endif // ABC_FIFO_ABC_FIFO_IMPL

#endif // SDL_INIT_EVERYTHING
#ifndef SDL_INIT_EVERYTHING
#undef SDL_sem
#undef SDL_atomic_t
#undef SDL_Thread
#endif

