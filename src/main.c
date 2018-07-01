//Using SDL and standard IO
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <wren.h>

// Constants
// Screen dimension constants
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 240;
const int SCREEN_WIDTH = GAME_WIDTH * 2;
const int SCREEN_HEIGHT = GAME_HEIGHT * 2;

// Game code
#include "io.c"
#include "engine.c"
#include "vm.c"

int main(int argc, char* args[])
{
  int result = EXIT_SUCCESS;
  WrenVM* vm = NULL;
  char* gameFile;

  //Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  if (argc == 2) {
    gameFile = readEntireFile(args[1]);
  } else {
    printf("No entry path was provided.\n");
    printf("Usage: ./dome [entryPath]\n");
    result = EXIT_FAILURE;
    goto cleanup;
  }

  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup; 
  };

  // Configure Wren VM
  vm = WREN_create();
  WrenInterpretResult interpreterResult = wrenInterpret(vm, gameFile);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }
  // Load the class into slot 0.

  WrenHandle* initMethod = wrenMakeCallHandle(vm, "init()");
  WrenHandle* updateMethod = wrenMakeCallHandle(vm, "update(_)");
  wrenEnsureSlots(vm, 1); 
  wrenGetVariable(vm, "main", "Game", 0); 
  WrenHandle* gameClass = wrenGetSlotHandle(vm, 0);

  // Initiate game loop
  wrenSetSlotHandle(vm, 0, gameClass);
  interpreterResult = wrenCall(vm, initMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }
  bool running = true;
  SDL_Event event;
  while (running) {

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
      }
    }
    double elapsedTime = 0;
    wrenSetSlotHandle(vm, 0, gameClass);
    wrenSetSlotDouble(vm, 1, elapsedTime);
    interpreterResult = wrenCall(vm, updateMethod);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      result = EXIT_FAILURE;
      goto cleanup;
    }

    // clear screen
    SDL_SetRenderDrawColor( engine.renderer, 0x00, 0x00, 0x00, 0x00 );
    SDL_RenderClear( engine.renderer );
    // Flip Buffer to Screen
    SDL_UpdateTexture(engine.texture, 0, engine.pixels, GAME_WIDTH * 4);
    SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);
  }

  wrenReleaseHandle(vm, updateMethod);
  wrenReleaseHandle(vm, gameClass);

cleanup:
  // Free resources
  WREN_free(vm);
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  } 

  return result;
}

