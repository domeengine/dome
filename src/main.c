#define _DEFAULT_SOURCE
#define NOMINMAX


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifndef DOME_VERSION
#define DOME_VERSION "0.0.0 - CUSTOM"
#endif

// Standard libs
#ifdef __MINGW32__
#include <windows.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <wren.h>
#include <SDL.h>
#include <vendor.h>

// Import plugin-specific definitions
#define WIN_EXPORT
#include "dome.h"
// project-specific definitions
#define external DOME_EXPORT
#define internal DOME_INTERNAL static
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
global_variable WrenHandle* keyboardClass = NULL;
global_variable WrenHandle* mouseClass = NULL;
global_variable WrenHandle* gamepadClass = NULL;
global_variable WrenHandle* updateInputMethod = NULL;

// These are set by cmd arguments
#ifdef DEBUG
global_variable bool DEBUG_MODE = true;
#else
global_variable bool DEBUG_MODE = false;
#endif
global_variable size_t AUDIO_BUFFER_SIZE = 2048;



// Game code
#include "math.c"
#include "strings.c"

#include "modules/map.c"

#include "plugin.h"
#include "engine.h"
#include "font/font8x8.h"
#include "io.c"

#include "audio/engine.h"
#include "audio/hashmap.c"
#include "audio/engine.c"
#include "audio/channel.c"
#include "audio/api.c"
#include "debug.c"

#include "engine.c"
#include "plugin.c"

#ifndef __EMSCRIPTEN__
#include "tools/help.c"
#include "tools/fuse.c"
#include "tools/embed.c"
#include "tools/nest.c"
#endif

#include "modules/dome.c"
#include "modules/font.c"
#include "modules/io.c"
#include "modules/audio.c"
#include "modules/graphics.c"
#include "modules/image.c"
#include "modules/input.c"
#include "modules/json.c"
#include "modules/platform.c"
#include "modules/random.c"
#include "modules/plugin.c"
// Comes last to register modules
#include "vm.c"

#include "game.c"


internal void
printTitle(ENGINE* engine) {
  ENGINE_printLog(engine, "DOME - Design-Oriented Minimalist Engine\n");
}

internal void
printVersion(ENGINE* engine) {
  ENGINE_printLog(engine, "Version: " DOME_VERSION " - " DOME_HASH "\n");
  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  ENGINE_printLog(engine, "SDL version: %d.%d.%d (Compiled)\n", compiled.major, compiled.minor, compiled.patch);
  ENGINE_printLog(engine, "SDL version %d.%d.%d (Linked)\n", linked.major, linked.minor, linked.patch);
  ENGINE_printLog(engine, "Wren version: %d.%d.%d\n", WREN_VERSION_MAJOR, WREN_VERSION_MINOR, WREN_VERSION_PATCH);

  ENGINE_printLog(engine, "\n");
}


internal void
printUsage(ENGINE* engine) {
  ENGINE_printLog(engine, "\nUsage: \n");

  ENGINE_printLog(engine, "  dome [options] [<command>]\n");
  ENGINE_printLog(engine, "  dome [options] [--] <entry_path> [<argument>]...\n");
  ENGINE_printLog(engine, "  dome -h | --help\n");
  ENGINE_printLog(engine, "  dome -v | --version\n");
  ENGINE_printLog(engine, "\nAvailable Commands: \n");
  ENGINE_printLog(engine, "  embed    Converts a Wren source file to a C include file for plugin development.\n");
  ENGINE_printLog(engine, "  fuse     Merges a bundle with DOME to make a standalone file.\n");
  ENGINE_printLog(engine, "  help     Displays information on how to use each command.\n");
  ENGINE_printLog(engine, "  nest     Bundle a project into a single file.\n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -b --buffer=<buf>   Set the audio buffer size (default: 11)\n");
#ifdef __MINGW32__
  ENGINE_printLog(engine, "  -c --console        Opens a console window for development.\n");
#endif
  ENGINE_printLog(engine, "  -d --debug          Enables debug mode.\n");
  ENGINE_printLog(engine, "  -h --help           Show this screen.\n");
  ENGINE_printLog(engine, "  -v --version        Show version.\n");
}

int main(int argc, char* args[])
{
  // configuring the buffer has to be first

  setbuf(stdout, NULL);
  setvbuf(stdout, NULL, _IONBF, 0);
  setbuf(stderr, NULL);
  setvbuf(stderr, NULL, _IONBF, 0);


  int result = EXIT_SUCCESS;
  WrenVM* vm = NULL;
  size_t gameFileLength;
  char* gameFile;
  INIT_TO_ZERO(ENGINE, engine);

#ifdef __EMSCRIPTEN__
  emscripten_wget("game.egg", "game.egg");
#endif
  INIT_TO_ZERO(LOOP_STATE, loop);
  loop.FPS = 60;
  loop.MS_PER_FRAME = ceil(1000.0 / loop.FPS);

  ENGINE_init(&engine);
  engine.argv = calloc(max(2, argc), sizeof(char*));
  engine.argv[0] = args[0];
  engine.argv[1] = NULL;
  loop.engine = &engine;

#ifndef __EMSCRIPTEN__
  // Is this web? If No...
  // Are we running in fused mode?
  // Did we get a path?
  // No - Try opening game.egg, or main.wren
  // Yes - Try opening it. If it's an egg, run it, otherwise don't.

  char* binaryPath = FUSE_getExecutablePath();
  if (binaryPath == NULL) {
    ENGINE_printLog(&engine, "dome: Could not allocate memory. Aborting.\n");
    result = EXIT_FAILURE;
    goto cleanup;
  }
  // Check if end of file has marker
  FILE* self = fopen(binaryPath, "rb");
  if (self == NULL) {
    ENGINE_printLog(&engine, "dome: Could not read binary: %s\n", strerror(errno));
    result = EXIT_FAILURE;
    goto cleanup;
  }
  int fileResult = fseek (self, -((long int)sizeof(DOME_FUSED_HEADER)), SEEK_END);
  if (fileResult != 0) {
    ENGINE_printLog(&engine, "dome: Could not introspect binary: %s\n", strerror(errno));
    result = EXIT_FAILURE;
    goto cleanup;
  }
  DOME_FUSED_HEADER header;
  fileResult = fread(&header, sizeof(DOME_FUSED_HEADER), 1, self);
  if (fileResult != 1) {
    ENGINE_printLog(&engine, "dome: Could not introspect binary: %s\n", strerror(errno));
    fclose(self);
    result = EXIT_FAILURE;
    goto cleanup;
  }

  if (memcmp("DOME", header.magic1, 4) == 0 && memcmp("DOME", header.magic2, 4) == 0) {
    if (header.version == 1) {
      engine.tar = malloc(sizeof(mtar_t));
      FUSE_open(engine.tar, self, header.offset);
      engine.fused = true;
    } else {
      ENGINE_printLog(&engine, "dome: Fused mode data is in the wrong format.");
      fclose(self);
      result = EXIT_FAILURE;
      goto cleanup;
    }
  } else {
    // We aren't in fused mode.
    fclose(self);
  }
  free(binaryPath);

  struct optparse_long longopts[] = {
    {"help", 'h', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},
#ifdef __MINGW32__
    {"console", 'c', OPTPARSE_NONE},
#endif
    {"buffer", 'b', OPTPARSE_REQUIRED},
    {"debug", 'd', OPTPARSE_NONE},
    {0}
  };

  int option;
  char **subargv = NULL;
  struct optparse options;
  optparse_init(&options, args);
  options.permute = 0;

  if (!engine.fused) {
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
      switch (option) {
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
        case 'c':
          {
            AllocConsole();
            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
          } break;
#endif
        case 'd':
          {
            DEBUG_MODE = true;
            ENGINE_printLog(&engine, "Debug Mode enabled\n");
          } break;
        case 'h':
          {
            printTitle(&engine);
            printUsage(&engine);
            goto cleanup;
          }
        case 'v':
          {
            printTitle(&engine);
            printVersion(&engine);
            goto cleanup;
          }
        case '?':
          {
            fprintf(stderr, "%s: %s\n", args[0], options.errmsg);
            result = EXIT_FAILURE;
            goto cleanup;
          }
      }
    }

    if (argc > 1) {
      static const struct {
        char name[8];
        char letter;
        int (*cmd)(ENGINE*, char **);
      } cmds[] = {
        {"fuse", 'f',  FUSE_perform },
        {"embed", 'e', EMBED_perform },
        {"help",  'h', HELP_perform },
        {"nest", 'n', NEST_perform }
      };
      int ncmds = sizeof(cmds) / sizeof(*cmds);
      subargv = args + options.optind;
      // If we match a subcommand, execute it
      for (int i = 0; i < ncmds; i++) {
        if (!strncmp(cmds[i].name, subargv[0], strlen(subargv[0]))) {
          result = cmds[i].cmd(&engine, subargv);
          goto cleanup;
        }
      }
    }
  }

  // we are trying to play the game
  // Get non-option args list
  int domeArgCount = 1;
  char* otherArg = NULL;
  while ((otherArg = optparse_arg(&options))) {
    engine.argv[domeArgCount] = strdup(otherArg);
    domeArgCount++;
  }
  // This has to be here because we modify the value shortly.
  bool autoResolve = (domeArgCount == 1);
  domeArgCount = max(2, domeArgCount);
  engine.argv = realloc(engine.argv, sizeof(char*) * domeArgCount);
  engine.argc = domeArgCount;

#else
  engine.argc = 1;
  bool autoResolve = true;
  char* entryArgument = NULL;
#endif

  char* defaultEggName = "game.egg";
  char* mainFileName = "main.wren";
  char* finalFileName = NULL;
  char entryPath[PATH_MAX];
  char* base = BASEPATH_get();
  bool resolved = false;
  char* entryArgument = NULL;
  if (domeArgCount > 1) {
    entryArgument = engine.argv[1];
  }

  if (engine.fused) {
    strcpy(entryPath, mainFileName);
  } else {
    // Set the basepath according to the incoming argument
    if (entryArgument != NULL) {
      strcpy(entryPath, base);
      strcat(entryPath, entryArgument);
      if (isDirectory(entryPath)) {
        autoResolve = true;
        BASEPATH_set(entryPath);
      } else {
        autoResolve = false;
        char* directory = dirname(strdup(entryPath));
        finalFileName = basename(strdup(entryPath));
        BASEPATH_set(directory);
        free(directory);
      }

      base = BASEPATH_get();
      chdir(base);
    }

    if (autoResolve) {
      strcpy(entryPath, base);
      strcat(entryPath, defaultEggName);
    }

    if (doesFileExist(entryPath)) {
      mtar_t* tar = malloc(sizeof(mtar_t));
      int tarResult = mtar_open(tar, entryPath, "r");
      if (tarResult == MTAR_ESUCCESS) {
        engine.tar = tar;
        finalFileName = NULL;
        resolved = true;
        ENGINE_printLog(&engine, "Loading bundle %s\n", entryPath);
      } else {
        // Not a valid tar file.
        free(tar);
      }
    }

    if (engine.tar == NULL) {
      if (autoResolve) {
        strcpy(entryPath, base);
        strcat(entryPath, mainFileName);
        finalFileName = NULL;
      }
      resolved = true;
    }
  }

  if (!engine.fused && !resolved) {
    result = EXIT_FAILURE;
    goto cleanup;
  } else {
    free(entryArgument);
    engine.argv[1] = strdup(entryPath);
    strcpy(entryPath, finalFileName == NULL ? mainFileName : finalFileName);
  }

  // The basepath is incorporated later, so we pass the basename version to this method.
  gameFile = ENGINE_readFile(&engine, entryPath, &gameFileLength);
  if (gameFile == NULL) {
    if (engine.tar != NULL) {
      ENGINE_printLog(&engine, "Error: Could not load %s in bundle.\n", entryPath);
    } else {
      ENGINE_printLog(&engine, "Error: Could not load %s.\n", entryPath);
    }
    printUsage(&engine);
    result = EXIT_FAILURE;
    goto cleanup;
  }

  result = ENGINE_start(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup;
  }

  // Configure Wren VM
  vm = VM_create(&engine);
  WrenInterpretResult interpreterResult;
  loop.vm = vm;

  // Load user game file
  WrenHandle* initMethod = NULL;

  interpreterResult = wrenInterpret(vm, "main", gameFile);
  free(gameFile);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto vm_cleanup;
  }
  // Load the class into slot 0.


  wrenEnsureSlots(vm, 3);
  initMethod = wrenMakeCallHandle(vm, "init()");
  wrenGetVariable(vm, "main", "Game", 0);
  loop.gameClass = wrenGetSlotHandle(vm, 0);
  loop.updateMethod = wrenMakeCallHandle(vm, "update()");
  loop.drawMethod = wrenMakeCallHandle(vm, "draw(_)");

  SDL_SetRenderDrawColor(engine.renderer, 0x00, 0x00, 0x00, 0xFF);

  // Initiate game loop

  wrenSetSlotHandle(vm, 0, loop.gameClass);
  interpreterResult = wrenCall(vm, initMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    result = EXIT_FAILURE;
    goto vm_cleanup;
  }
  // Release this handle if it finished successfully
  wrenReleaseHandle(vm, initMethod);
  initMethod = NULL;
  engine.initialized = true;

  SDL_SetWindowPosition(engine.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(engine.window);

  loop.lag = loop.MS_PER_FRAME;
  result = DOME_processInput(&loop);
  if (result != EXIT_SUCCESS) {
    goto vm_cleanup;
  }
  loop.windowBlurred = false;
  loop.previousTime = SDL_GetPerformanceCounter();
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(DOME_loop, &loop, 0, true);
#endif
  while (engine.running) {

    // processInput()
    if (loop.windowBlurred) {
      result = DOME_processInput(&loop);
      if (result != EXIT_SUCCESS) {
        goto vm_cleanup;
      }
    }

    loop.currentTime = SDL_GetPerformanceCounter();
    loop.elapsed = 1000 * (loop.currentTime - loop.previousTime) / (double) SDL_GetPerformanceFrequency();
    loop.previousTime = loop.currentTime;


    // If we aren't focused, we skip the update loop and let the CPU sleep
    // to be good citizens
    if (loop.windowBlurred) {
      SDL_Delay(50);
      continue;
    }

    if(fabs(loop.elapsed - 1.0/120.0) < .0002){
      loop.elapsed = 1.0/120.0;
    }
    if(fabs(loop.elapsed - 1.0/60.0) < .0002){
      loop.elapsed = 1.0/60.0;
    }
    if(fabs(loop.elapsed - 1.0/30.0) < .0002){
      loop.elapsed = 1.0/30.0;
    }
    loop.lag += loop.elapsed;

    if (engine.lockstep) {
      if (loop.lag >= loop.MS_PER_FRAME) {
        result = DOME_processInput(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        result = DOME_update(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        result = DOME_render(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        loop.lag = mid(0, loop.lag - loop.MS_PER_FRAME, loop.MS_PER_FRAME);
        DOME_flip(&loop);
      }
    } else {
      loop.attempts = 5;
      while (loop.attempts > 0 && loop.lag >= loop.MS_PER_FRAME) {
        loop.attempts--;

        result = DOME_processInput(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        // update()
        result = DOME_update(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        loop.lag -= loop.MS_PER_FRAME;
      }
      // render();
      result = DOME_render(&loop);
      if (result != EXIT_SUCCESS) {
        goto vm_cleanup;
      }
      if (loop.attempts == 0) {
        loop.lag = 0;
      }
      DOME_flip(&loop);
    }


    if (!engine.vsyncEnabled) {
      SDL_Delay(1);
    }
  }

vm_cleanup:
  // Finish processing async threads so we can release resources
  ENGINE_finishAsync(&engine);
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    if (event.type == SDL_USEREVENT) {
      if (event.user.code == EVENT_LOAD_FILE) {
        FILESYSTEM_loadEventComplete(&event);
      }
    }
  }

  // Free resources
  ENGINE_reportError(&engine);

  if (initMethod != NULL) {
    wrenReleaseHandle(vm, initMethod);
  }

  DOME_release(&loop);

  if (bufferClass != NULL) {
    wrenReleaseHandle(vm, bufferClass);
  }

  INPUT_release(vm);

  AUDIO_ENGINE_halt(engine.audioEngine);
  AUDIO_ENGINE_releaseHandles(engine.audioEngine, vm);

  // TODO: test if this is in the right place
  result = engine.exit_status;

cleanup:
  BASEPATH_free();
  VM_free(vm);
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}
