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
global_variable size_t GIF_SCALE = 1;



// Game code
#include "math.c"
#include "strings.c"

#include "modules/map.c"

#include "plugin.h"
#include "engine.h"
#include "util/font8x8.h"
#include "io.c"

#include "audio/engine.h"
#include "audio/hashmap.c"
#include "audio/engine.c"
#include "audio/channel.c"
#include "audio/api.c"
#include "debug.c"

#include "engine.c"
#include "plugin.c"

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
#include "util/wrenembed.c"


// Comes last to register modules
#include "vm.c"

typedef struct {
  ENGINE* engine;
  WrenVM* vm;
  WrenHandle* gameClass;
  WrenHandle* updateMethod;
  WrenHandle* drawMethod;
  double MS_PER_FRAME;
  double FPS;
  double lag;
  uint64_t previousTime;
  uint64_t currentTime;
  double elapsed;
  bool windowBlurred;
  uint8_t attempts;
  bool tickRender;
} LOOP_STATE;

internal void
LOOP_release(LOOP_STATE* state) {
  WrenVM* vm = state->vm;

  if (state->drawMethod != NULL) {
    wrenReleaseHandle(vm, state->drawMethod);
  }

  if (state->updateMethod != NULL) {
    wrenReleaseHandle(vm, state->updateMethod);
  }

  if (state->gameClass != NULL) {
    wrenReleaseHandle(vm, state->gameClass);
  }
}

internal int
LOOP_processInput(LOOP_STATE* state) {
  WrenInterpretResult interpreterResult;
  ENGINE* engine = state->engine;
  WrenVM* vm = state->vm;
  engine->mouse.scrollX = 0;
  engine->mouse.scrollY = 0;
  SDL_Event event;
  INPUT_clearText(vm);
  while(SDL_PollEvent(&event)) {
    switch (event.type)
    {
      case SDL_QUIT:
        engine->running = false;
        break;
      case SDL_WINDOWEVENT:
        {
          if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
              event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            SDL_RenderGetViewport(engine->renderer, &(engine->viewport));
            break;
          }
          if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
#ifdef __EMSCRIPTEN__
            AUDIO_ENGINE_pause(engine->audioEngine);
#endif
            state->windowBlurred = true;
          } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
#ifdef __EMSCRIPTEN__
            AUDIO_ENGINE_resume(engine->audioEngine);
#endif
            ENGINE_updateTextRegion(engine);
            state->windowBlurred = false;
          }
        } break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        {
          SDL_Keycode keyCode = event.key.keysym.sym;
          if (keyCode == SDLK_F3 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
            engine->debugEnabled = !engine->debugEnabled;
          } else if (keyCode == SDLK_F2 && event.key.state == SDL_PRESSED && event.key.repeat == 0) {
            ENGINE_takeScreenshot(engine);
          } else if (event.key.repeat == 0) {
            char* buttonName = strToLower((char*)SDL_GetKeyName(keyCode));
            interpreterResult = INPUT_update(vm, DOME_INPUT_KEYBOARD, buttonName, event.key.state == SDL_PRESSED);
            free(buttonName);
            if (interpreterResult != WREN_RESULT_SUCCESS) {
              return EXIT_FAILURE;
            }
          }
        } break;
      case SDL_TEXTEDITING:
        {
          if (utf8len(event.edit.text) > 0) {
            INPUT_setCompositionText(vm, event.edit.text, event.edit.start, event.edit.length);
          }
        } break;
      case SDL_TEXTINPUT:
        {
          if (utf8len(event.text.text) > 0) {
            INPUT_addText(vm, event.text.text);
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
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
        {
          SDL_ControllerButtonEvent cbutton = event.cbutton;
          const char* buttonName = GAMEPAD_stringFromButton(cbutton.button);
          interpreterResult = GAMEPAD_eventButtonPressed(vm, cbutton.which, buttonName, cbutton.state == SDL_PRESSED);
          if (interpreterResult != WREN_RESULT_SUCCESS) {
            return EXIT_FAILURE;
          }
        } break;
      case SDL_MOUSEWHEEL:
        {
          int dir = event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1;
          engine->mouse.scrollX += event.wheel.x * dir;
          // Down should be positive to match the direction of rendering.
          engine->mouse.scrollY += event.wheel.y * -dir;
        } break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        {
          char* buttonName;
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: buttonName = "left"; break;
            case SDL_BUTTON_MIDDLE: buttonName = "middle"; break;
            case SDL_BUTTON_RIGHT: buttonName = "right"; break;
            case SDL_BUTTON_X1: buttonName = "x1"; break;
            default:
            case SDL_BUTTON_X2: buttonName = "x2"; break;
          }
          bool state = event.button.state == SDL_PRESSED;
          interpreterResult = INPUT_update(vm, DOME_INPUT_MOUSE, buttonName, state);
          if (interpreterResult != WREN_RESULT_SUCCESS) {
            return EXIT_FAILURE;
          }
        } break;
      case SDL_USEREVENT:
        {
          ENGINE_printLog(engine, "Event code %i\n", event.user.code);
          if (event.user.code == EVENT_LOAD_FILE) {
            FILESYSTEM_loadEventComplete(&event);
          }
        }
    }
  }
  if (inputCaptured) {
    interpreterResult = INPUT_commit(vm);
    if (interpreterResult != WREN_RESULT_SUCCESS) {
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

internal int
LOOP_render(LOOP_STATE* state) {
  if (PLUGIN_COLLECTION_runHook(state->engine, DOME_PLUGIN_HOOK_PRE_DRAW) != DOME_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  };
  WrenInterpretResult interpreterResult;
  wrenEnsureSlots(state->vm, 8);
  wrenSetSlotHandle(state->vm, 0, state->gameClass);
  wrenSetSlotDouble(state->vm, 1, ((double)state->lag / state->MS_PER_FRAME));
  interpreterResult = wrenCall(state->vm, state->drawMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  }
  if (PLUGIN_COLLECTION_runHook(state->engine, DOME_PLUGIN_HOOK_POST_DRAW) != DOME_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  };


  return EXIT_SUCCESS;
}

internal void
LOOP_flip(LOOP_STATE* state) {

  state->engine->debug.elapsed = state->elapsed;
  if (state->engine->debugEnabled) {
    ENGINE_drawDebug(state->engine);
  }
  // Flip Buffer to Screen
  SDL_UpdateTexture(state->engine->texture, 0, state->engine->canvas.pixels, state->engine->canvas.width * 4);
  // Flip buffer for recording
  if (state->engine->record.makeGif) {
    size_t imageSize = state->engine->canvas.width * state->engine->canvas.height * 4;
    memcpy(state->engine->record.gifPixels, state->engine->canvas.pixels, imageSize);
  }
  // clear screen
  SDL_RenderClear(state->engine->renderer);
  SDL_RenderCopy(state->engine->renderer, state->engine->texture, NULL, NULL);
  SDL_RenderPresent(state->engine->renderer);
}

internal int
LOOP_update(LOOP_STATE* state) {
  WrenInterpretResult interpreterResult;

  if (PLUGIN_COLLECTION_runHook(state->engine, DOME_PLUGIN_HOOK_PRE_UPDATE) != DOME_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  };

  wrenEnsureSlots(state->vm, 8);
  wrenSetSlotHandle(state->vm, 0, state->gameClass);
  interpreterResult = wrenCall(state->vm, state->updateMethod);
  if (interpreterResult != WREN_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  }
  if (PLUGIN_COLLECTION_runHook(state->engine, DOME_PLUGIN_HOOK_POST_UPDATE) != DOME_RESULT_SUCCESS) {
    return EXIT_FAILURE;
  };
  // updateAudio()
  AUDIO_ENGINE_update(state->engine->audioEngine, state->vm);
  return EXIT_SUCCESS;
}

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

  ENGINE_printLog(engine, "  dome [options]\n");
  ENGINE_printLog(engine, "  dome [options] [--] entry_path [arguments]\n");
  ENGINE_printLog(engine, "  dome -e | --embed sourceFile [moduleName] [destinationFile]\n");
  ENGINE_printLog(engine, "  dome -h | --help\n");
  ENGINE_printLog(engine, "  dome -v | --version\n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -b --buffer=<buf>   Set the audio buffer size (default: 11)\n");
#ifdef __MINGW32__
  ENGINE_printLog(engine, "  -c --console        Opens a console window for development.\n");
#endif
  ENGINE_printLog(engine, "  -d --debug          Enables debug mode.\n");
  ENGINE_printLog(engine, "  -e --embed          Converts a Wren source file to a C include file.\n");
  ENGINE_printLog(engine, "  -h --help           Show this screen.\n");
  ENGINE_printLog(engine, "  -r --record=<gif>   Record video to <gif>.\n");
  ENGINE_printLog(engine, "  -v --version        Show version.\n");
}

void DOME_loop(void* data) {
  LOOP_STATE loop = *((LOOP_STATE*)data);
  loop.currentTime = SDL_GetPerformanceCounter();
  loop.elapsed = 1000 * (loop.currentTime - loop.previousTime) / (double) SDL_GetPerformanceFrequency();
  loop.previousTime = loop.currentTime;
  loop.lag += loop.elapsed;

  if (loop.lag >= loop.MS_PER_FRAME) {
    LOOP_processInput(&loop);
    if (loop.windowBlurred) {
      loop.lag = 0;
      loop.tickRender = true;
      return;
    }
    LOOP_update(&loop);
    if (loop.tickRender) {
      LOOP_render(&loop);
      LOOP_flip(&loop);
    }
    loop.tickRender = !loop.tickRender;
    loop.lag = mid(0, loop.lag - loop.MS_PER_FRAME, loop.MS_PER_FRAME);
  }
  *((LOOP_STATE*)data) = loop;
}


int fuse(int argc, char* args[])
{
  if (argc < 2) {
    fputs("Not enough arguments\n", stderr);
    return EXIT_FAILURE;
  }

  char* fileName = args[2];
  char* binaryPath = getExecutablePath();
  if (binaryPath != NULL) {
    // Check if end of file has marker

    FILE* binary = fopen(binaryPath, "ab");
    int result = fseek (binary, -sizeof(DOME_EGG_HEADER), SEEK_END);
    DOME_EGG_HEADER header;
    result = fread(&header, sizeof(DOME_EGG_HEADER), 1, binary);
    fclose(binary);

    if (result == 1) {
      if (strncmp("DOME", header.magic2, 4) == 0) {
        printf("This copy of DOME is already fused to an EGG file. Please use a fresh instance.");
        return EXIT_FAILURE;
      }
    }
    printf("Fusing...");
    binary = fopen(binaryPath, "ab");
    FILE* egg = fopen(fileName, "rb");
    if (egg == NULL) {
      printf("Error: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
    int c;
    uint64_t size = 0;
    while((c = fgetc(egg)) != EOF) {
      fputc(c, binary);
      size++;
      printf("%c", c);
    }

    strncpy(header.magic1, "DOME", 4);
    strncpy(header.magic2, "DOME", 4);
    header.version = 1;
    header.offset = size;
    fwrite(&header, sizeof(DOME_EGG_HEADER), 1, binary);
    fclose(binary);
    fclose(egg);
  }
  free(binaryPath);
  return EXIT_SUCCESS;
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
  engine.record.gifName = "test.gif";
  engine.record.makeGif = false;
  INIT_TO_ZERO(LOOP_STATE, loop);
  loop.FPS = 60;
  loop.MS_PER_FRAME = ceil(1000.0 / loop.FPS);

  ENGINE_init(&engine);
  loop.engine = &engine;

  struct optparse_long longopts[] = {
    {"buffer", 'b', OPTPARSE_REQUIRED},
    #ifdef __MINGW32__
    {"console", 'c', OPTPARSE_NONE},
    #endif
    {"debug", 'd', OPTPARSE_NONE},
    {"embed", 'e', OPTPARSE_NONE},
    {"help", 'h', OPTPARSE_NONE},
    {"record", 'r', OPTPARSE_OPTIONAL},
    {"scale", 's', OPTPARSE_REQUIRED},
    {"fuse", 'f', OPTPARSE_REQUIRED},
    {"version", 'v', OPTPARSE_NONE},
    {0}
  };

  int option;
  struct optparse options;
  optparse_init(&options, args);
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
        ENGINE_printLog(&engine, "Debug Mode enabled\n");
        break;
      case 'e':
        WRENEMBED_encodeAndDumpInDOME(argc, args);
        goto cleanup;
      case 'f':
        fuse(argc, args);
        goto cleanup;
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

    char pathBuf[PATH_MAX];
    char* fileName = NULL;

    // Get non-option args list
    engine.argv = calloc(max(2, argc), sizeof(char*));
    engine.argv[0] = args[0];
    engine.argv[1] = NULL;
    int domeArgCount = 1;
    char* otherArg = NULL;
    while ((otherArg = optparse_arg(&options))) {
      engine.argv[domeArgCount] = otherArg;
      domeArgCount++;
    }

    bool autoResolve = (domeArgCount == 1);

    domeArgCount = max(2, domeArgCount);
    engine.argv = realloc(engine.argv, sizeof(char*) * domeArgCount);
    engine.argc = domeArgCount;

    char* arg = NULL;
    if (domeArgCount > 1) {
      arg = engine.argv[1];
    }

    // Get establish the path components: filename(?) and basepath.
    if (arg != NULL) {
      strcpy(pathBuf, base);
      strcat(pathBuf, arg);
      if (isDirectory(pathBuf)) {
        BASEPATH_set(pathBuf);
        autoResolve = true;
      } else {
        char* dirc = strdup(pathBuf);
        char* basec = strdup(pathBuf);
        // This sets the filename used.
        fileName = strdup(basename(dirc));
        BASEPATH_set(dirname(basec));
        free(dirc);
        free(basec);
      }

      base = BASEPATH_get();
    }

    // If a filename is given in the path, use it, or assume its 'game.egg'
    strcpy(pathBuf, base);
    strcat(pathBuf, !autoResolve ? fileName : defaultEggName);
    chdir(base);

    if (doesFileExist(pathBuf)) {
      // the current path exists, let's see if it's a TAR file.
      engine.tar = malloc(sizeof(mtar_t));
      int tarResult = mtar_open(engine.tar, pathBuf, "r");
      if (tarResult == MTAR_ESUCCESS) {
        ENGINE_printLog(&engine, "Loading bundle %s\n", pathBuf);
        engine.argv[1] = strdup(pathBuf);
      } else {
        // Not a valid tar file.
        free(engine.tar);
        engine.tar = NULL;
      }
    } else {
      printf("DOME didn't find a base file here\n");
      char* binaryPath = getExecutablePath();
      if (binaryPath != NULL) {
        // Check if end of file has marker
        FILE* self = fopen(binaryPath, "rb");
        int result = fseek (self, -sizeof(DOME_EGG_HEADER), SEEK_END);
        if (result == 0) {
          DOME_EGG_HEADER header;
          result = fread(&header, sizeof(DOME_EGG_HEADER), 1, self);
          if (result == 1) {
            if (strncmp("DOME", header.magic2, 4) == 0) {
              printf("Header found!\n");
            } else {
              printf("'%s'\n", header.magic2);
            }
          }
        }
        fclose(self);
      }
      free(binaryPath);
    }

    if (engine.tar != NULL) {
      // It is a tar file, we need to look for a "main.wren" entry point.
      strcpy(pathBuf, mainFileName);
    } else {
      // Not a tar file, use the given path or main.wren
      strcpy(pathBuf, base);
      strcat(pathBuf, !autoResolve ? fileName : mainFileName);
      engine.argv[1] = strdup(pathBuf);
      strcpy(pathBuf, !autoResolve ? fileName : mainFileName);
    }

    if (fileName != NULL) {
      free(fileName);
    }

    // The basepath is incorporated later, so we pass the basename version to this method.
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

  result = ENGINE_start(&engine);
  if (result == EXIT_FAILURE) {
    goto cleanup;
  }

  // Configure Wren VM
  vm = VM_create(&engine);
  WrenInterpretResult interpreterResult;
  loop.vm = vm;

  // Load user game file
  SDL_Thread* recordThread = NULL;

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

  // Resizing from init must happen before we begin recording
  if (engine.record.makeGif) {
    recordThread = SDL_CreateThread(ENGINE_record, "DOMErecorder", &engine);
  }
  loop.lag = loop.MS_PER_FRAME;
  result = LOOP_processInput(&loop);
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
      result = LOOP_processInput(&loop);
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
        result = LOOP_processInput(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        result = LOOP_update(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        result = LOOP_render(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        loop.lag = mid(0, loop.lag - loop.MS_PER_FRAME, loop.MS_PER_FRAME);
        LOOP_flip(&loop);
      }
    } else {
      loop.attempts = 5;
      while (loop.attempts > 0 && loop.lag >= loop.MS_PER_FRAME) {
        loop.attempts--;

        result = LOOP_processInput(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        // update()
        result = LOOP_update(&loop);
        if (result != EXIT_SUCCESS) {
          goto vm_cleanup;
        }
        loop.lag -= loop.MS_PER_FRAME;
      }
      // render();
      result = LOOP_render(&loop);
      if (result != EXIT_SUCCESS) {
        goto vm_cleanup;
      }
      if (loop.attempts == 0) {
        loop.lag = 0;
      }
      LOOP_flip(&loop);
    }


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

  LOOP_release(&loop);

  if (bufferClass != NULL) {
    wrenReleaseHandle(vm, bufferClass);
  }

  INPUT_release(vm);

  AUDIO_ENGINE_halt(engine.audioEngine);
  AUDIO_ENGINE_releaseHandles(engine.audioEngine, vm);

cleanup:
  BASEPATH_free();
  VM_free(vm);
  result = engine.exit_status;
  ENGINE_free(&engine);
  //Quit SDL subsystems
  if (strlen(SDL_GetError()) > 0) {
    SDL_Quit();
  }

  return result;
}
