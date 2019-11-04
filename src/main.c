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
#include <setjmp.h>

#include <wren.h>
#include <SDL2/SDL.h>
#include <jo_gif.h>

#include <microtar/microtar.h>
#include <microtar/microtar.c>


// Set up STB_IMAGE
#define STBI_NO_STDIO
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

// We need this here so it can be used by the DOME module
global_variable jmp_buf loop_exit;
// Used in the io variable, but we need to catch it here
global_variable WrenHandle* bufferClass = NULL;

// Game code
#include "math.c"
#include "debug.c"
/*
#include "util/font.c"
*/
#include "util/font8x8.h"
#include "io.c"
#include "map.c"
#include "modules/modules.c"
#include "engine.c"
#include "modules/dome.c"
#include "modules/io.c"
#include "modules/audio.c"
#include "modules/graphics.c"
#include "modules/image.c"
#include "modules/input.c"
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

  // Load user game file
  interpreterResult = wrenInterpret(vm, "main", gameFile);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto cleanup;
  }
  // Load the class into slot 0.

  wrenEnsureSlots(vm, 3);
  WrenHandle* initMethod = wrenMakeCallHandle(vm, "init()");
  WrenHandle* updateMethod = wrenMakeCallHandle(vm, "update()");
  WrenHandle* drawMethod = wrenMakeCallHandle(vm, "draw(_)");
  wrenGetVariable(vm, "main", "Game", 0);
  WrenHandle* gameClass = wrenGetSlotHandle(vm, 0);
  wrenGetVariable(vm, "audio", "AudioEngine", 0);
  WrenHandle* audioEngineClass = wrenGetSlotHandle(vm, 0);
  if (bufferClass == NULL)
  {
    wrenGetVariable(vm, "io", "DataBuffer", 1);
    bufferClass = wrenGetSlotHandle(vm, 1);
  }

  // Initiate game loop
  uint8_t FPS = 60;
  double MS_PER_FRAME = (1000.0 / FPS);

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
  SDL_Event event;
  SDL_SetRenderDrawColor( engine.renderer, 0x00, 0x00, 0x00, 0x00 );
  SDL_RenderClear(engine.renderer);
  double avgFps = FPS;
  while (engine.running) {
    uint32_t currentTime = SDL_GetTicks();
    int32_t elapsed = currentTime - previousTime;
    previousTime = currentTime;
    lag += elapsed;

    // processInput()
    while(SDL_PollEvent(&event)) {
      switch (event.type)
      {
        case SDL_QUIT:
          engine.running = false;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          {
            SDL_Keycode keyCode = event.key.keysym.sym;
            if (keyCode == SDLK_F2 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
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
      wrenEnsureSlots(vm, 8);
      wrenSetSlotHandle(vm, 0, gameClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto cleanup;
      }
      // updateAudio()
      wrenEnsureSlots(vm, 3);
      wrenSetSlotHandle(vm, 0, audioEngineClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto cleanup;
      }
      lag -= MS_PER_FRAME;
    }

    // render();
    wrenEnsureSlots(vm, 8);
    wrenSetSlotHandle(vm, 0, gameClass);
    wrenSetSlotDouble(vm, 1, ((double)lag / MS_PER_FRAME));
    interpreterResult = wrenCall(vm, drawMethod);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      result = EXIT_FAILURE;
      goto cleanup;
    }

    char buffer[20];
    // Choose alpha depending on how fast or slow you want old averages to decay.
    // 0.9 is usually a good choice.
    double framesThisSecond = 1000.0 / (elapsed+1);
    static double alpha = 0.9;
    avgFps = alpha * avgFps + (1.0 - alpha) * framesThisSecond;
    snprintf(buffer, sizeof(buffer), "%.01f fps", avgFps);   // here 2 means binary
    ENGINE_print(&engine, buffer, GAME_WIDTH - 4*8, GAME_HEIGHT - 8, 0xFFFFFFFF);
    // SDL_SetWindowTitle(engine.window, buffer);

    // Flip Buffer to Screen
    SDL_UpdateTexture(engine.texture, 0, engine.pixels, GAME_WIDTH * 4);
    // clear screen
    SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);




    elapsed = SDL_GetTicks() - currentTime;
    result = setjmp(loop_exit);
  }
  if (makeGif) {
    jo_gif_end(&gif);
  }

  wrenReleaseHandle(vm, audioEngineClass);
  wrenReleaseHandle(vm, initMethod);
  wrenReleaseHandle(vm, drawMethod);
  wrenReleaseHandle(vm, updateMethod);
  wrenReleaseHandle(vm, gameClass);

  if (bufferClass != NULL) {
    wrenReleaseHandle(vm, bufferClass);
  }

cleanup:
  // Free resources
  // TODO: Lock the Audio Engine here.
  AUDIO_ENGINE_halt(engine.audioEngine);
  VM_free(vm);
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}

