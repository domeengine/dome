//Using SDL and standard IO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libgen.h>

#include <wren.h>
#include <SDL2/SDL.h>

// Set up STB_IMAGE #define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"


#define STB_VORBIS_NO_PUSHDATA_API
#include "include/stb_vorbis.c"

#define ABC_FIFO_IMPL
#include "include/ABC_fifo.h"

#define internal static
#define global_variable static
#define local_persist static

#define INIT_TO_ZERO(Type, name)\
  Type name;\
  memset(&name, 0, sizeof(Type));

// Constants
// Screen dimension constants
const int16_t GAME_WIDTH = 320;
const int16_t GAME_HEIGHT = 240;
const int16_t SCREEN_WIDTH = GAME_WIDTH * 2;
const int16_t SCREEN_HEIGHT = GAME_HEIGHT * 2;
const int32_t FPS = 60;
const int32_t MS_PER_FRAME = 1000 / FPS;

// Game code
#include "math.c"
#include "debug.c"
#include "util/font.c"
#include "map.c"
#include "io.c"
#include "engine.c"
#include "engine/io.c"
#include "engine/audio.c"
#include "engine/image.c"
#include "engine/point.c"
#include "vm.c"

int main(int argc, char* args[])
{
  int result = EXIT_SUCCESS;
  WrenVM* vm = NULL;
  char* gameFile;

  //Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  INIT_TO_ZERO(ENGINE, engine);

  if (argc == 2) {
    gameFile = readEntireFile(args[1]);
  } else {
    printf("No entry path was provided.\n");
    printf("Usage: ./dome [entry path]\n");
    result = EXIT_FAILURE;
    goto cleanup;
  }

  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup;
  };

  char* initFile = readEntireFile("src/engine/init.wren");
  char* completeGameFile = calloc(strlen(initFile) + strlen(gameFile), sizeof(char));
  strcpy(completeGameFile, initFile);
  strcpy(completeGameFile+strlen(initFile), gameFile);

  // Configure Wren VM
  vm = VM_create(&engine);
  WrenInterpretResult interpreterResult = wrenInterpret(vm, completeGameFile);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }
  // Load the class into slot 0.

  WrenHandle* initMethod = wrenMakeCallHandle(vm, "init()");
  WrenHandle* updateMethod = wrenMakeCallHandle(vm, "update()");
  WrenHandle* drawMethod = wrenMakeCallHandle(vm, "draw(_)");
  wrenEnsureSlots(vm, 2);
  wrenGetVariable(vm, "main", "Game", 0);
  WrenHandle* gameClass = wrenGetSlotHandle(vm, 0);
  wrenGetVariable(vm, "main", "AudioEngine_internal", 0);
  WrenHandle* audioEngineClass = wrenGetSlotHandle(vm, 0);

  // Initiate game loop
  wrenSetSlotHandle(vm, 0, gameClass);
  interpreterResult = wrenCall(vm, initMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }

  SDL_ShowWindow(engine.window);

  uint32_t previousTime = SDL_GetTicks();
  int32_t lag = 0;
  bool running = true;
  SDL_Event event;
  SDL_SetRenderDrawColor( engine.renderer, 0x00, 0x00, 0x00, 0x00 );
  while (running) {
    int32_t currentTime = SDL_GetTicks();
    int32_t elapsed = currentTime - previousTime;
    previousTime = currentTime;
    lag += elapsed;

    // processInput()
    while(SDL_PollEvent(&event)) {
      switch (event.type)
      {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          {
            SDL_Keycode keyCode = event.key.keysym.sym;
            if(keyCode == SDLK_ESCAPE && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
              // TODO: Let Wren decide when to end game
              running = false;
            } else {
              ENGINE_storeKeyState(&engine, keyCode, event.key.state);
            }
          } break;
        case SDL_USEREVENT:
          {
            printf("Event code %i\n", event.user.code);
            if (event.user.code == EVENT_LOAD_FILE) {
              GAMEFILE_loadComplete(event.user.data1, event.user.data2);
            }
          }
      }
    }

    // Decouple updates from rendering
    uint8_t attempts = 0;
    while (lag >= MS_PER_FRAME && attempts < 10) {
      wrenSetSlotHandle(vm, 0, gameClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto cleanup;
      }
      lag -= MS_PER_FRAME;
      attempts += 1;
    }

    // Update audio system
    wrenSetSlotHandle(vm, 0, audioEngineClass);
    interpreterResult = wrenCall(vm, updateMethod);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      result = EXIT_FAILURE;
      goto cleanup;
    }

    // render();
    wrenSetSlotHandle(vm, 0, gameClass);
    wrenSetSlotDouble(vm, 1, (double)lag / MS_PER_FRAME);
    interpreterResult = wrenCall(vm, drawMethod);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      result = EXIT_FAILURE;
      goto cleanup;
    }

    // Flip Buffer to Screen
    SDL_UpdateTexture(engine.texture, 0, engine.pixels, GAME_WIDTH * 4);
    // clear screen
    SDL_RenderClear(engine.renderer);
    SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);
    elapsed = SDL_GetTicks() - currentTime;
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "DOME - %.02f fps", 1000.0 / (elapsed+1));   // here 2 means binary
    SDL_SetWindowTitle(engine.window, buffer);
  }

  wrenReleaseHandle(vm, initMethod);
  wrenReleaseHandle(vm, drawMethod);
  wrenReleaseHandle(vm, updateMethod);
  wrenReleaseHandle(vm, gameClass);
  wrenReleaseHandle(vm, audioEngineClass);

cleanup:
  // Free resources
  VM_free(vm);
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}

