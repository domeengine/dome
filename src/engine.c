typedef struct {
  bool isPressed; 
} KEY_STATE;

typedef struct {
  KEY_STATE left;
  KEY_STATE right;
  KEY_STATE up;
  KEY_STATE down;
} INPUT_STATE;
typedef struct {
  SDL_Window* window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  void* pixels;
  INPUT_STATE keyboard;
} ENGINE;

ENGINE engine = {0};
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


void ENGINE_free(ENGINE* engine) {
  if (engine == NULL) {
    return;
  }

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

void ENGINE_pset(ENGINE* engine, int16_t x, int16_t y, uint32_t c) {
  // Draw pixel at (x,y)
  if (0 <= x && x < GAME_WIDTH && 0 <= y && y < GAME_HEIGHT) {
    ((uint32_t*)(engine->pixels))[GAME_WIDTH * y + x] = c; 
  }
}

void ENGINE_storeKeyState(ENGINE* engine, SDL_Keycode keycode, uint8_t state) {
  if(keycode == SDLK_LEFT) {
    engine->keyboard.left.isPressed = (state == SDL_PRESSED);
  }
  if(keycode == SDLK_RIGHT) {
    engine->keyboard.right.isPressed = (state == SDL_PRESSED);
  }
  if(keycode == SDLK_UP) {
    engine->keyboard.up.isPressed = (state == SDL_PRESSED);
  }
  if(keycode == SDLK_DOWN) {
    engine->keyboard.down.isPressed = (state == SDL_PRESSED);
  }
}

KEY_STATE ENGINE_getKeyState(ENGINE* engine, SDL_Keycode keycode) {
  if(keycode == SDLK_LEFT) {
    return engine->keyboard.left;
  }
  if(keycode == SDLK_RIGHT) {
    return engine->keyboard.right;
  }
  if(keycode == SDLK_UP) {
    return engine->keyboard.up;
  }
  if(keycode == SDLK_DOWN) {
    return engine->keyboard.down;
  }
  KEY_STATE none;
  none.isPressed = false;
  return none;
}
