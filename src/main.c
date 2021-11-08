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

char* resolveEntryPath(ENGINE* engine, char* entryArgument, bool autoResolve) {
  char* defaultEggName = "game.egg";
  char* mainFileName = "main.wren";
  char* finalFileName = NULL;
  char* entryPath = malloc(sizeof(char) * PATH_MAX);
  char* base = BASEPATH_get();
  bool resolved = false;

  if (engine->fused) {
    strcpy(entryPath, mainFileName);
  } else {
    if (entryArgument != NULL) {
      // Set the basepath according to the incoming argument
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
#ifdef __EMSCRIPTEN__
  emscripten_wget("game.egg", "game.egg");
#endif
    }

    if (doesFileExist(entryPath)) {
      mtar_t* tar = malloc(sizeof(mtar_t));
      int tarResult = mtar_open(tar, entryPath, "r");
      if (tarResult == MTAR_ESUCCESS) {
        engine->tar = tar;
        finalFileName = NULL;
        resolved = true;
        ENGINE_printLog(engine, "Loading bundle %s\n", entryPath);
      } else {
        // Not a valid tar file.
        free(tar);
      }
    }

    if (engine->tar == NULL) {
      if (autoResolve) {
        strcpy(entryPath, base);
        strcat(entryPath, mainFileName);
        finalFileName = NULL;
        resolved = doesFileExist(entryPath);
      } else {
        resolved = true;
      }
    }
  }

  if (!engine->fused && !resolved) {
    ENGINE_printLog(engine, "Error: Could not find an entry point at: %s\n", dirname(entryPath));
    printUsage(engine);
    return NULL;
  } else {
    free(entryArgument);
    engine->argv[1] = strdup(entryPath);
    strcpy(entryPath, finalFileName == NULL ? mainFileName : finalFileName);
  }
  return entryPath;
}


int main(int argc, char* argv[])
{
  // configuring the buffer has to be first

  setbuf(stdout, NULL);
  setvbuf(stdout, NULL, _IONBF, 0);
  setbuf(stderr, NULL);
  setvbuf(stderr, NULL, _IONBF, 0);

  int result = EXIT_SUCCESS;
  INIT_TO_ZERO(ENGINE, engine);

  ENGINE_init(&engine);
  engine.argv = calloc(max(2, argc), sizeof(char*));
  engine.argv[0] = argv[0];
  engine.argv[1] = NULL;
  bool autoResolve = true;

#ifndef __EMSCRIPTEN__
  result = FUSE_introspectBinary(&engine);
  if (result != EXIT_SUCCESS) {
    goto cleanup;
  }

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
  optparse_init(&options, argv);
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
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
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
      subargv = argv + options.optind;
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
  // This has to be here because we modify the value of domeArgCount after
  // to account for a strict 2-arg requirement. (See dome.Process documentation)
  autoResolve = (domeArgCount == 1);
  domeArgCount = max(2, domeArgCount);
  engine.argv = realloc(engine.argv, sizeof(char*) * domeArgCount);
  engine.argc = domeArgCount;
#endif

  char* entryPath = resolveEntryPath(&engine, engine.argv[1], autoResolve);
  if (entryPath == NULL) {
    goto cleanup;
  }

  result = DOME_begin(&engine, entryPath);
  free(entryPath);

cleanup:
  BASEPATH_free();
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}

