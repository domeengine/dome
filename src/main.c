// TODO: We need this for realpath in BSD, but it won't be available in windows (_fullpath)
#define _DEFAULT_SOURCE

#define DOME_VERSION "1.0.0-alpha"

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
#include <SDL.h>
#include <jo_gif.h>

#if DOME_OPT_FFI
#include <ffi.h>
#endif

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

#include <microtar/microtar.h>
#include <microtar/microtar.c>

// Set up STB_IMAGE
#define STBI_FAILURE_USERMSG
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNG
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

#define STRINGS_EQUAL(a, b) (strcmp(a, b) == 0)

#define VM_ABORT(vm, error) do {\
  wrenSetSlotString(vm, 0, error);\
  wrenAbortFiber(vm, 0); \
} while(false);


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
#include "strings.c"
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
#if DOME_OPT_FFI
#include "modules/ffi.c"
#endif
#include "modules/io.c"
#include "modules/audio.c"
#include "modules/graphics.c"
#include "modules/image.c"
#include "modules/input.c"
#include "vm.c"

internal void
printTitle(void) {
  printf("DOME - Dynamic Opinionated Mini Engine\n");
}

internal void
printVersion(void) {
  printf("Version: " DOME_VERSION " - " HASH"\n");
  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  printf("SDL version: %d.%d.%d (Compiled)\n", compiled.major, compiled.minor, compiled.patch);
  printf("SDL version %d.%d.%d (Linked)\n", linked.major, linked.minor, linked.patch);

#if DOME_OPT_FFI
  printf("FFI module is available");
#else
  printf("FFI module is unavailable");
#endif
}


internal void
printUsage(void) {

  printf("\nUsage: \n");
  printf("  dome [entry path]\n");
  printf("  dome --record=<gif> [entry path]\n");
  printf("  dome -h | --help\n");
  printf("  dome -v | --version\n");
  printf("\nOptions: \n");
  printf("  -h --help          Show this screen.\n");
  printf("  -v --version       Show version.\n");
  printf("  -r --record=<gif>  Record video to <gif>.\n");

}

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
  struct optparse_long longopts[] = {
    {"help", 'h', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},
    {"record", 'r', OPTPARSE_NONE},
    {0}
  };
  // char *arg;
  int option;
  struct optparse options;
  optparse_init(&options, args);
  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 'h':
        printTitle();
        printUsage();
        goto cleanup;
      case 'v':
        printTitle();
        printVersion();
        goto cleanup;
      case 'r':
        makeGif = true;
        if (options.optarg != NULL) {
          gifName = options.optarg;
        } else {
          gifName = "dome.gif";
        }
        printf("GIF Recording is enabled: Saving to %s\n", gifName);
        break;
      case '?':
        fprintf(stderr, "%s: %s\n", args[0], options.errmsg);
        result = EXIT_FAILURE;
        goto cleanup;
    }
  }

  {
    char* fileName = "game.egg";
    char* mainFileName = "main.wren";
    char* base = SDL_GetBasePath();
    char* arg = optparse_arg(&options);
    if (arg != NULL) {
      fileName = arg;
    }
    char pathBuf[strlen(base)+strlen(fileName)+1];
    strcpy(pathBuf, base);
    strcat(pathBuf, fileName);
    SDL_free(base);

    if (doesFileExist(pathBuf)) {
      engine.tar = malloc(sizeof(mtar_t));
      int tarResult = mtar_open(engine.tar, pathBuf, "r");
      if (tarResult != MTAR_ESUCCESS) {
        free(engine.tar);
        engine.tar = NULL;
        fileName = arg;
      } else {
        printf("Loading bundle %s\n", pathBuf);
        fileName = mainFileName;
      }
    } else if (arg == NULL) {
      fileName = mainFileName;
    }
    gameFile = ENGINE_readFile(&engine, fileName, &gameFileLength);
    if (gameFile == NULL) {
      if (engine.tar != NULL) {
        printf("Error: Could not load main.wren in bundle.\n\n");
      } else if (arg == NULL) {
        printf("Error: Could not find a default entry path.\n\n");
      } else {
        printf("Error: %s does not exist.\n", arg);
      }
      printUsage();
      result = EXIT_FAILURE;
      goto cleanup;
    }
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
  free(gameFile);
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
  double MS_PER_FRAME = ceil(1000.0 / FPS);

  if (setjmp(loop_exit) == 0) {
    wrenSetSlotHandle(vm, 0, gameClass);
    interpreterResult = wrenCall(vm, initMethod);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      result = EXIT_FAILURE;
      goto vm_cleanup;
    }
    SDL_ShowWindow(engine.window);
    SDL_SetRenderDrawColor( engine.renderer, 0x00, 0x00, 0x00, 0xFF);
  }

  jo_gif_t gif;
  size_t imageSize = engine.width * engine.height;
  uint8_t t = 0;
  uint8_t* destroyableImage = NULL;
  if (makeGif) {
    destroyableImage = (uint8_t*)malloc(imageSize*4*sizeof(uint8_t));
    gif = jo_gif_start(gifName, engine.width, engine.height, 0, 31);
  }

  uint64_t previousTime = SDL_GetPerformanceCounter();
  int32_t lag = 0;
  SDL_Event event;
  if (setjmp(loop_exit) == 0) {

    while (engine.running) {

      // processInput()
      while(SDL_PollEvent(&event)) {
        switch (event.type)
        {
          case SDL_QUIT:
            engine.running = false;
            break;
          case SDL_WINDOWEVENT:
            {
              if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_RenderGetViewport(engine.renderer, &(engine.viewport));
              }
            } break;
          case SDL_KEYDOWN:
          case SDL_KEYUP:
            {
              SDL_Keycode keyCode = event.key.keysym.sym;
              if (keyCode == SDLK_F3 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
                engine.debugEnabled = !engine.debugEnabled;
              } else if (keyCode == SDLK_F2 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
                ENGINE_takeScreenshot(&engine);
              }
            } break;
          case SDL_CONTROLLERDEVICEADDED:
            {
              GAMEPAD_eventAdded(vm, event.cdevice.which);
            } break;
          case SDL_CONTROLLERDEVICEREMOVED:
            {
              GAMEPAD_eventRemoved(vm, event.cdevice.which);
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

      uint64_t currentTime = SDL_GetPerformanceCounter();
      int32_t elapsed = 1000 * (currentTime - previousTime) / SDL_GetPerformanceFrequency();
      previousTime = currentTime;

      if(fabs(elapsed - 1.0/120.0) < .0002){
        elapsed = 1.0/120.0;
      }
      if(fabs(elapsed - 1.0/60.0) < .0002){
        elapsed = 1.0/60.0;
      }
      if(fabs(elapsed - 1.0/30.0) < .0002){
        elapsed = 1.0/30.0;
      }
      lag += elapsed;

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

      while (lag > MS_PER_FRAME) {
        wrenEnsureSlots(vm, 8);
        wrenSetSlotHandle(vm, 0, gameClass);
        interpreterResult = wrenCall(vm, updateMethod);
        if (interpreterResult != WREN_RESULT_SUCCESS) {
          result = EXIT_FAILURE;
          goto vm_cleanup;
        }
        // updateAudio()
        wrenEnsureSlots(vm, 3);
        wrenSetSlotHandle(vm, 0, audioEngineClass);
        interpreterResult = wrenCall(vm, updateMethod);
        if (interpreterResult != WREN_RESULT_SUCCESS) {
          result = EXIT_FAILURE;
          goto vm_cleanup;
        }
        lag -= MS_PER_FRAME;

        if (engine.lockstep) {
          lag = mid(0, lag, MS_PER_FRAME);
          break;
        }
      }

      // render();
      wrenEnsureSlots(vm, 8);
      wrenSetSlotHandle(vm, 0, gameClass);
      wrenSetSlotDouble(vm, 1, ((double)lag / MS_PER_FRAME));
      interpreterResult = wrenCall(vm, drawMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto vm_cleanup;
      }

      if (engine.debugEnabled) {
        engine.debug.elapsed = elapsed;
        ENGINE_drawDebug(&engine);
      }

      // Flip Buffer to Screen
      SDL_UpdateTexture(engine.texture, 0, engine.pixels, engine.width * 4);

      // clear screen
      SDL_RenderClear(engine.renderer);
      SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
      SDL_RenderPresent(engine.renderer);

      if (!engine.vsyncEnabled) {
        SDL_Delay(1);
      }
    }
  }

vm_cleanup:
  if (makeGif) {
    jo_gif_end(&gif);
    if (destroyableImage != NULL) {
      free(destroyableImage);
    }
  }

  // Finish processing async threads so we can release resources
  ENGINE_finishAsync(&engine);
  while(SDL_PollEvent(&event)) {
    if (event.type == SDL_USEREVENT) {
      if (event.user.code == EVENT_LOAD_FILE) {
        FILESYSTEM_loadEventComplete(&event);
      }
    }
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

