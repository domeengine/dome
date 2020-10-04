#define _DEFAULT_SOURCE
#define NOMINMAX

#ifndef DOME_VERSION
#define DOME_VERSION "1.0.0-alpha"
#endif

// Standard libs
#ifdef __MINGW32__
#include <windows.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <tinydir.h>
#include <utf8.h>

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <libgen.h>


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

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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

#define ASSERT_SLOT_TYPE(vm, slot, type, fieldName) \
  if (wrenGetSlotType(vm, slot) != WREN_TYPE_##type) { \
    VM_ABORT(vm, #fieldName " was not " #type); \
    return; \
  }



// Constants
// Screen dimension constants
#define GAME_WIDTH 320
#define GAME_HEIGHT 240
#define SCREEN_WIDTH GAME_WIDTH * 2
#define SCREEN_HEIGHT GAME_HEIGHT * 2

// Used in the io variable, but we need to catch it here
global_variable WrenHandle* bufferClass = NULL;
global_variable WrenHandle* audioEngineClass = NULL;

// These are set by cmd arguments
#ifdef DEBUG
global_variable bool DEBUG_MODE = true;
#else
global_variable bool DEBUG_MODE = false;
#endif
global_variable size_t AUDIO_BUFFER_SIZE = 2048;
global_variable size_t GIF_SCALE = 1;

// Game code
#include "math.c"
#include "strings.c"
#include "audio_types.c"
#include "modules/map.c"
#include "engine.h"
#include "debug.c"
/*
#include "util/font.c"
*/
#include "util/font8x8.h"
#include "io.c"
#include "engine.c"
#include "modules/dome.c"
#if DOME_OPT_FFI
#include "modules/ffi.c"
#endif
#include "modules/font.c"
#include "modules/io.c"
#include "modules/audio.c"
#include "modules/graphics.c"
#include "modules/image.c"
#include "modules/input.c"
#include "vm.c"

internal void
printTitle(ENGINE* engine) {
  ENGINE_printLog(engine, "DOME - Dynamic Opinionated Minimalist Engine\n");
}

internal void
printVersion(ENGINE* engine) {
  ENGINE_printLog(engine, "Version: " DOME_VERSION " - " HASH"\n");
  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  ENGINE_printLog(engine, "SDL version: %d.%d.%d (Compiled)\n", compiled.major, compiled.minor, compiled.patch);
  ENGINE_printLog(engine, "SDL version %d.%d.%d (Linked)\n", linked.major, linked.minor, linked.patch);

#if DOME_OPT_FFI
  ENGINE_printLog(engine, "FFI module is available");
#else
  ENGINE_printLog(engine, "FFI module is unavailable");
#endif
}


internal void
printUsage(ENGINE* engine) {
  ENGINE_printLog(engine, "\nUsage: \n");
  ENGINE_printLog(engine, "  dome [-c] [-d | --debug] [-r<gif> | --record=<gif>] [-b<buf> | --buffer=<buf>] [entry path]\n");
  ENGINE_printLog(engine, "  dome -h | --help\n");
  ENGINE_printLog(engine, "  dome -v | --version\n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -b --buffer=<buf>   Set the audio buffer size (default: 11)\n");
#ifdef __MINGW32__
  ENGINE_printLog(engine, "  -c --console        Opens a console window for development.\n");
#endif
  ENGINE_printLog(engine, "  -d --debug          Enables debug mode.\n");
  ENGINE_printLog(engine, "  -h --help           Show this screen.\n");
  ENGINE_printLog(engine, "  -v --version        Show version.\n");
  ENGINE_printLog(engine, "  -r --record=<gif>   Record video to <gif>.\n");
}

int main(int argc, char* args[])
{

#if defined _WIN32
  SDL_setenv("SDL_AUDIODRIVER", "directsound", true);
#endif

  int result = EXIT_SUCCESS;
  WrenVM* vm = NULL;
  size_t gameFileLength;
  char* gameFile;
  INIT_TO_ZERO(ENGINE, engine);
  engine.record.gifName = "test.gif";
  engine.record.makeGif = false;

  //Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    ENGINE_printLog(&engine, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto cleanup;
  }

  result = ENGINE_init(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup;
  };

  // TODO: Use getopt to parse the arguments better
  struct optparse_long longopts[] = {
    {"buffer", 'b', OPTPARSE_REQUIRED},
    #ifdef __MINGW32__
    {"console", 'c', OPTPARSE_NONE},
    #endif
    {"debug", 'd', OPTPARSE_NONE},
    {"help", 'h', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},
    {"record", 'r', OPTPARSE_OPTIONAL},
    {"scale", 's', OPTPARSE_REQUIRED},
    {0}
  };
  // char *arg;
  int option;
  struct optparse options;
  optparse_init(&options, args);

  engine.argv = args;
  engine.argc = argc;

  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 's':
        {
          int scale = atoi(options.optarg);
          if (scale <= 0) {
            // If it wasn't valid, set to a meaningful default.
            GIF_SCALE = 1;
          }
          GIF_SCALE = scale;
        } break;
      case 'b':
        {
          int shift = atoi(options.optarg);
          if (shift == 0) {
            // If it wasn't valid, set to a meaningful default.
            AUDIO_BUFFER_SIZE = 2048;
          }
          AUDIO_BUFFER_SIZE = 1 << shift;
        } break;
#ifdef __MINGW32__
      case 'c': {
          AllocConsole();
          freopen("CONIN$", "r", stdin);
          freopen("CONOUT$", "w", stdout);
          freopen("CONOUT$", "w", stderr);
      } break;
#endif
      case 'd':
        DEBUG_MODE = true;
        ("Debug Mode enabled\n");
        break;
      case 'h':
        printTitle(&engine);
        printUsage(&engine);
        goto cleanup;
      case 'r':
        engine.record.makeGif = true;
        if (options.optarg != NULL) {
          engine.record.gifName = options.optarg;
        } else {
          engine.record.gifName = "dome.gif";
        }
        ENGINE_printLog(&engine, "GIF Recording is enabled: Saving to %s\n", engine.record.gifName);
        break;
      case 'v':
        printTitle(&engine);
        printVersion(&engine);
        goto cleanup;
      case '?':
        fprintf(stderr, "%s: %s\n", args[0], options.errmsg);
        result = EXIT_FAILURE;
        goto cleanup;
    }
  }

  {
    char* defaultEggName = "game.egg";
    char* mainFileName = "main.wren";

    char* base = BASEPATH_get();
    char* arg = optparse_arg(&options);

    char pathBuf[PATH_MAX];

    char* fileName = NULL;

    if (arg != NULL) {
      strcpy(pathBuf, base);
      strcat(pathBuf, arg);
      if (isDirectory(pathBuf)) {
        BASEPATH_set(pathBuf);
      } else {
        char* dirc = strdup(pathBuf);
        char* basec = strdup(pathBuf);
        fileName = basename(dirc);
        BASEPATH_set(dirname(basec));
        free(dirc);
        free(basec);
      }

      base = BASEPATH_get();
    }

    strcpy(pathBuf, base);
    strcat(pathBuf, fileName ? fileName : defaultEggName);

    if (doesFileExist(pathBuf)) {
      engine.tar = malloc(sizeof(mtar_t));
      int tarResult = mtar_open(engine.tar, pathBuf, "r");
      if (tarResult == MTAR_ESUCCESS) {
        ENGINE_printLog(&engine, "Loading bundle %s\n", pathBuf);
      } else {
        free(engine.tar);
        engine.tar = NULL;
      }
    }

    if (engine.tar != NULL) {
      strcpy(pathBuf, mainFileName);
    } else {
      strcpy(pathBuf, fileName ? fileName : mainFileName);
    }

    gameFile = ENGINE_readFile(&engine, pathBuf, &gameFileLength);
    if (gameFile == NULL) {
      if (engine.tar != NULL) {
        ENGINE_printLog(&engine, "Error: Could not load %s in bundle.\n", pathBuf);
      } else {
        ENGINE_printLog(&engine, "Error: Could not load %s.\n", pathBuf);
      }
      printUsage(&engine);
      result = EXIT_FAILURE;
      goto cleanup;
    }
  }


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

  // Initiate game loop
  uint8_t FPS = 60;
  double MS_PER_FRAME = ceil(1000.0 / FPS);
  SDL_Thread* recordThread = NULL;

  wrenSetSlotHandle(vm, 0, gameClass);
  interpreterResult = wrenCall(vm, initMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto vm_cleanup;
  }
  engine.initialized = true;


  SDL_ShowWindow(engine.window);
  SDL_SetRenderDrawColor(engine.renderer, 0x00, 0x00, 0x00, 0xFF);

  // Resizing from init must happen before we begin recording
  if (engine.record.makeGif) {
    recordThread = SDL_CreateThread(ENGINE_record, "DOMErecorder", &engine);
  }
  uint64_t previousTime = SDL_GetPerformanceCounter();
  int32_t lag = 0;
  bool windowHasFocus = false;
  SDL_Event event;
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
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
              AUDIO_ENGINE_pause(engine.audioEngine);
              windowHasFocus = true;
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
              AUDIO_ENGINE_resume(engine.audioEngine);
              windowHasFocus = false;
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
            ENGINE_printLog(&engine, "Event code %i\n", event.user.code);
            if (event.user.code == EVENT_LOAD_FILE) {
              FILESYSTEM_loadEventComplete(&event);
            }
          }
      }
    }

    uint64_t currentTime = SDL_GetPerformanceCounter();
    int32_t elapsed = 1000 * (currentTime - previousTime) / SDL_GetPerformanceFrequency();
    previousTime = currentTime;

    // If we aren't focused, we skip the update loop and let the CPU sleep
    // to be good citizens
    if (windowHasFocus) {
      SDL_Delay(50);
      continue;
    }

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

    while (lag > MS_PER_FRAME) {
      wrenEnsureSlots(vm, 8);
      wrenSetSlotHandle(vm, 0, gameClass);
      interpreterResult = wrenCall(vm, updateMethod);
      if (interpreterResult != WREN_RESULT_SUCCESS) {
        result = EXIT_FAILURE;
        goto vm_cleanup;
      }
      // updateAudio()
      if (audioEngineClass != NULL) {
        wrenEnsureSlots(vm, 3);
        wrenSetSlotHandle(vm, 0, audioEngineClass);
        AUDIO_ENGINE_lock(engine.audioEngine);
        interpreterResult = wrenCall(vm, updateMethod);
        AUDIO_ENGINE_unlock(engine.audioEngine);
        if (interpreterResult != WREN_RESULT_SUCCESS) {
          result = EXIT_FAILURE;
          goto vm_cleanup;
        }
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
    // Flip buffer for recording
    if (engine.record.makeGif) {
      size_t imageSize = engine.width * engine.height * 4;
      memcpy(engine.record.gifPixels, engine.pixels, imageSize);
    }

    // clear screen
    SDL_RenderClear(engine.renderer);
    SDL_RenderCopy(engine.renderer, engine.texture, NULL, NULL);
    SDL_RenderPresent(engine.renderer);

    if (!engine.vsyncEnabled) {
      SDL_Delay(1);
    }
  }

vm_cleanup:

  if (recordThread != NULL) {
    SDL_WaitThread(recordThread, NULL);
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

  wrenReleaseHandle(vm, initMethod);
  wrenReleaseHandle(vm, drawMethod);
  wrenReleaseHandle(vm, updateMethod);
  wrenReleaseHandle(vm, gameClass);

  if (bufferClass != NULL) {
    wrenReleaseHandle(vm, bufferClass);
  }

  if (audioEngineClass != NULL) {
    wrenReleaseHandle(vm, audioEngineClass);
  }

cleanup:
  // Free resources
  // TODO: Lock the Audio Engine here.
  ENGINE_reportError(&engine);
  BASEPATH_free();
  AUDIO_ENGINE_halt(engine.audioEngine);
  VM_free(vm);
  result = engine.exit_status;
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}

