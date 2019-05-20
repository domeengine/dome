// TODO: We need this for realpath in BSD, but it won't be available in windows (_fullpath)
#define _DEFAULT_SOURCE

// Standard libs
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <string.h>
#include <math.h>
#include <libgen.h>

#include <wren.h>
#include <SDL2/SDL.h>
#include <jo_gif.h>

#include <microtar/microtar.h>
#include <microtar/microtar.c>


// Set up STB_IMAGE
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// Setup STB_VORBIS
#define STB_VORBIS_NO_PUSHDATA_API
#include <stb_vorbis.c>

// Setup ABC_FIFO
#define ABC_FIFO_IMPL
#include <ABC_fifo.h>

#define internal static
#define global_variable static
#define local_persist static

#define INIT_TO_ZERO(Type, name)\
  Type name;\
  memset(&name, 0, sizeof(Type));

// Constants
// Screen dimension constants
#define GAME_WIDTH 320
#define GAME_HEIGHT 240
#define SCREEN_WIDTH GAME_WIDTH * 2
#define SCREEN_HEIGHT GAME_HEIGHT * 2
#define FPS 60
#define MS_PER_FRAME 1000 / FPS

// Game code
#include "math.c"
#include "debug.c"
#include "util/font.c"
#include "map.c"
#include "io.c"
#include "engine/modules.c"
#include "engine.c"
#include "engine/io.c"
#include "engine/audio.c"
#include "engine/image.c"
#include "engine/point.c"
#include "vm.c"

int main(int argc, char* args[])
{

  #if defined _WIN32
  SDL_setenv("SDL_AUDIODRIVER", "directsound", true);
  #endif

  bool makeGif = false;
  char* gifName = "test.gif";
  int result = EXIT_SUCCESS;
  WrenVM* vm = NULL;
  size_t gameFileLength;
  char* gameFile;
  INIT_TO_ZERO(ENGINE, engine);

  //Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  // TODO: Use getopt to parse the arguments better
  if (argc >= 2 && argc <= 3) {
    gameFile = ENGINE_readFile(&engine, args[1], &gameFileLength);
    if (gameFile == NULL) {
      printf("%s does not exist.\n", args[1]);
      result = EXIT_FAILURE;
      goto cleanup;
    }
    if (argc == 3) {
      printf("GIF Recording is enabled\n");
      makeGif = true;
      gifName = args[2];
    }
  } else {
    // Test for game.egg first
    char* base = SDL_GetBasePath();
    char* fileName = "game.egg";
    char* mainFileName = "main.wren";
    char pathBuf[strlen(base)+strlen(fileName)+1];
    strcpy(pathBuf, base);
    strcat(pathBuf, fileName);
    if (doesFileExist(pathBuf)) {
      printf("Loading bundle %s\n", pathBuf);
      engine.tar = malloc(sizeof(mtar_t));
      mtar_open(engine.tar, pathBuf, "r");
      gameFile = ENGINE_readFile(&engine, mainFileName, &gameFileLength);
    } else {
      fileName = mainFileName;
      char pathBuf[strlen(base)+strlen(fileName)+1];
      strcpy(pathBuf, base);
      strcat(pathBuf, fileName);
      gameFile = ENGINE_readFile(&engine, fileName, &gameFileLength);
      if (gameFile == NULL) {
        // file doesn't exist
        printf("No entry path was provided.\n");
        printf("Usage: ./dome [entry path]\n");
        result = EXIT_FAILURE;
        goto cleanup;
      }
    }
    SDL_free(base);
  }

  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup;
  };

  // Configure Wren VM
  vm = VM_create(&engine);
  WrenInterpretResult interpreterResult;

  // Run wren engine init()
  interpreterResult = wrenInterpret(vm, initModule);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }

  // Load user game file
  interpreterResult = wrenInterpret(vm, gameFile);
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

  jo_gif_t gif;
  size_t imageSize = engine.width*engine.height;
  uint8_t t = 0;
  uint8_t* destroyableImage = (uint8_t*)malloc(imageSize*4*sizeof(uint8_t));
  if (makeGif) {
    gif = jo_gif_start(gifName, engine.width, engine.height, 0, 31);
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
            } else if (keyCode == SDLK_F2 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
              for (size_t i = 0; i < imageSize; i++) {
                uint32_t c = ((uint32_t*)engine.pixels)[i];
                uint8_t a = (0xFF000000 & c) >> 24;
                uint8_t r = (0x00FF0000 & c) >> 16;
                uint8_t g = (0x0000FF00 & c) >> 8;
                uint8_t b = (0x000000FF & c);
                ((uint32_t*)destroyableImage)[i] = a << 24 | b << 16 | g << 8 | r;
              }
              stbi_write_png("screenshot.png", engine.width, engine.height, 4, destroyableImage, engine.width * 4);
            }
          } break;
        case SDL_USEREVENT:
          {
            printf("Event code %i\n", event.user.code);
            if (event.user.code == EVENT_LOAD_FILE) {
              FILESYSTEM_loadEventComplete(&event);
            }
          }
      }
    }

    // update()
    if (lag >= MS_PER_FRAME) {
      t++;
      if (makeGif && t > 3) {
        t = 0;
        for (size_t i = 0; i < imageSize; i++) {
          uint32_t c = ((uint32_t*)engine.pixels)[i];
          uint8_t a = (0xFF000000 & c) >> 24;
          uint8_t r = (0x00FF0000 & c) >> 16;
          uint8_t g = (0x0000FF00 & c) >> 8;
          uint8_t b = (0x000000FF & c);
          ((uint32_t*)destroyableImage)[i] = a << 24 | b << 16 | g << 8 | r;
        }
        jo_gif_frame(&gif, destroyableImage, 8, true);
      }
    }
    while (lag >= MS_PER_FRAME) {
      wrenSetSlotHandle(vm, 0, gameClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto cleanup;
      }
      // updateAudio()
      wrenSetSlotHandle(vm, 0, audioEngineClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto cleanup;
      }
      lag -= MS_PER_FRAME;
    }

    // render();
    wrenSetSlotHandle(vm, 0, gameClass);
    wrenSetSlotDouble(vm, 1, ((double)lag / MS_PER_FRAME));
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
    // char buffer[20];
    // snprintf(buffer, sizeof(buffer), "DOME - %.02f fps", 1000.0 / (elapsed+1));   // here 2 means binary
    // SDL_SetWindowTitle(engine.window, buffer);

    elapsed = SDL_GetTicks() - currentTime;
  }
  if (makeGif) {
    jo_gif_end(&gif);
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

