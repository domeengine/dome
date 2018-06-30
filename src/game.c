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
  char* gameFile;


  //Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  printf("%d", argc);
  if (argc == 2) {
    gameFile = readEntireFile(args[1]);
  } else {
    result = EXIT_FAILURE;
    goto cleanup;
  }

  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup; 
  };

  // Configure Wren VM
  WrenVM* vm = WREN_create();
  WrenInterpretResult wResult = wrenInterpret(vm, gameFile);

  /*
     uint16_t x = 0;
     uint16_t y = 0;
     */

  // Initiate game loop
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
            SDL_Keycode KeyCode = event.key.keysym.sym;
            if(KeyCode == SDLK_ESCAPE && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
              // TODO: Let Wren decide when to end game
              running = false; 
            }
            if(KeyCode == SDLK_RIGHT && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
              // x += 1;
            }
          } break;
      }
    }
    ENGINE_pset(&engine, 5, 5, 0xFFFF00FF);

    // clear screen
    SDL_SetRenderDrawColor( engine.renderer, 0x00, 0x00, 0x00, 0x00 );
    SDL_RenderClear( engine.renderer );
    // Flip Buffer to Screen
    SDL_UpdateTexture(engine.texture, 0, engine.pixels, GAME_WIDTH * 4);
    SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);
  }

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

