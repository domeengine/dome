//Using SDL and standard IO
#include <stdio.h>
#include <SDL2/SDL.h>
#include <wren.h>

//Screen dimension constants
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 240;
const int SCREEN_WIDTH = GAME_WIDTH * 2;
const int SCREEN_HEIGHT = GAME_HEIGHT * 2;


typedef struct {
  SDL_Window* window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  void* pixels;
} ENGINE;

// Debug output for VM
void WREN_write(WrenVM* vm, const char* text) {
  printf("%s", text);
}

void WREN_error( 
      WrenVM* vm,
      WrenErrorType type, 
      const char* module, 
      int line, 
      const char* message) {
  if (type == WREN_ERROR_COMPILE) {
    printf("%s:%d: %s\n", module, line, message);
  } else if (type == WREN_ERROR_RUNTIME) {
    printf("Runtime error: %s\n", message);
  } else if (type == WREN_ERROR_STACK_TRACE) {
    printf("  %d: %s\n", line, module);
  } 
}

int ENGINE_init(ENGINE* engine) {
  int result = EXIT_SUCCESS;
  engine->window = NULL; 
  engine->renderer = NULL;
  engine->texture = NULL;
  engine->pixels = NULL;

  //Create window
  engine->window = SDL_CreateWindow("DOME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if(engine->window == NULL)
  {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->renderer = SDL_CreateRenderer(engine->window, -1, SDL_RENDERER_ACCELERATED);
  if (engine->renderer == NULL)
  {
    SDL_Log("Could not create a renderer: %s", SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }
  SDL_RenderSetLogicalSize(engine->renderer, GAME_WIDTH, GAME_HEIGHT);
  SDL_RenderSetIntegerScale(engine->renderer, SDL_TRUE);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET , GAME_WIDTH, GAME_HEIGHT);
  if (engine->texture == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->pixels = malloc(GAME_WIDTH * GAME_HEIGHT * 4);
  if (engine->pixels == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

engine_init_end:
  return result; 
} 

void ENGINE_pset(ENGINE* engine, uint16_t x, uint16_t y, uint32_t c) {
  // Draw pixel at (x,y)
  ((uint32_t*)(engine->pixels))[GAME_WIDTH * y + x] = c; 
}

void ENGINE_free(ENGINE* engine) {
  if (engine->pixels != NULL) {
    free(engine->pixels);
  }

  if (engine->texture) {
    SDL_DestroyTexture(engine->texture);
  }

  if (engine->renderer != NULL) {
    SDL_DestroyRenderer(engine->renderer);
  }
  if (engine->window != NULL) {
    SDL_DestroyWindow(engine->window);
  }
}

int main(int argc, char* args[])
{
  int result = EXIT_SUCCESS;

  //Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  ENGINE engine;
  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup; 
  };

  // Configure Wren VM
  WrenConfiguration config; 
  wrenInitConfiguration(&config);
  config.writeFn = WREN_write; 
  config.errorFn = WREN_error; 
  WrenVM* vm = wrenNewVM(&config);
  WrenInterpretResult wResult = wrenInterpret(vm, "System.print(\"I am running in a VM!\")");

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
  wrenFreeVM(vm);
  ENGINE_free(&engine);
  //Quit SDL subsystems
  SDL_Quit();

  return result;
}

