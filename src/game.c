//Using SDL and standard IO
#include <wren.h>
#include <SDL2/SDL.h>
#include <stdio.h>

//Screen dimension constants
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 240;
const int SCREEN_WIDTH = GAME_WIDTH * 3;
const int SCREEN_HEIGHT = GAME_HEIGHT * 2;

int main(int argc, char* args[])
{
  // WrenConfiguration config; 
  // wrenInitConfiguration(&config);
  int result = EXIT_SUCCESS;

  //The window we'll be rendering to
  SDL_Window* window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *texture = NULL;
  void* pixels = NULL;

  //Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }
  //Create window
  window = SDL_CreateWindow("Ghost: Infiltration", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if(window == NULL)
  {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
  {
    SDL_Log("Could not create a renderer: %s", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }
  SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);
  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET , GAME_WIDTH, GAME_HEIGHT);
  if (texture == NULL) {
    result = EXIT_FAILURE;
    goto cleanup;
  }

  pixels = malloc(GAME_WIDTH * GAME_HEIGHT * 4);
  if (pixels == NULL) {
    result = EXIT_FAILURE;
    goto cleanup;
  }

  uint16_t x = 0;
  uint16_t y = 0;

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
      }
    }
    ((uint32_t*)pixels)[GAME_WIDTH * y + x/2] = 0xFF00FFFF; 
    // clear screen
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x00 );
    SDL_RenderClear( renderer );
    // Flip Buffer to Screen
    SDL_UpdateTexture(texture, 0, pixels, GAME_WIDTH * 4);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

cleanup:
  // Free resources
  if (pixels != NULL) {
    free(pixels);
  }

  if (texture) {
    SDL_DestroyTexture(texture);
  }

  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }
  if (window != NULL) {
    SDL_DestroyWindow(window);
  }

  //Quit SDL subsystems
  SDL_Quit();

  return result;
}

