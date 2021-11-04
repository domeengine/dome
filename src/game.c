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
DOME_release(LOOP_STATE* state) {
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
DOME_processInput(LOOP_STATE* state) {
  WrenInterpretResult interpreterResult;
  ENGINE* engine = state->engine;
  WrenVM* vm = state->vm;
  engine->mouse.scrollX = 0;
  engine->mouse.scrollY = 0;
  SDL_Event event;
  INPUT_clearText(vm);
  while(SDL_PollEvent(&event)) {
    switch (event.type) {
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
DOME_render(LOOP_STATE* state) {
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
DOME_flip(LOOP_STATE* state) {
  state->engine->debug.elapsed = state->elapsed;
  if (state->engine->debugEnabled) {
    ENGINE_drawDebug(state->engine);
  }
  // Flip Buffer to Screen
  SDL_UpdateTexture(state->engine->texture, 0, state->engine->canvas.pixels, state->engine->canvas.width * 4);
  // clear screen
  SDL_RenderClear(state->engine->renderer);
  SDL_RenderCopy(state->engine->renderer, state->engine->texture, NULL, NULL);
  SDL_RenderPresent(state->engine->renderer);
}

internal int
DOME_update(LOOP_STATE* state) {
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

void DOME_loop(void* data) {
  LOOP_STATE loop = *((LOOP_STATE*)data);
  loop.currentTime = SDL_GetPerformanceCounter();
  loop.elapsed = 1000 * (loop.currentTime - loop.previousTime) / (double) SDL_GetPerformanceFrequency();
  loop.previousTime = loop.currentTime;
  loop.lag += loop.elapsed;

  if (loop.lag >= loop.MS_PER_FRAME) {
    DOME_processInput(&loop);
    if (loop.windowBlurred) {
      loop.lag = 0;
      loop.tickRender = true;
      return;
    }
    DOME_update(&loop);
    if (loop.tickRender) {
      DOME_render(&loop);
      DOME_flip(&loop);
    }
    loop.tickRender = !loop.tickRender;
    loop.lag = mid(0, loop.lag - loop.MS_PER_FRAME, loop.MS_PER_FRAME);
  }
  *((LOOP_STATE*)data) = loop;
}
