internal void
getColorComponents(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
  *r = color & 0xFF;
  *g = (color & (0xFF << 8)) >> 8;
  *b = (color & (0xFF << 16)) >> 16;
}

internal void
ENGINE_openLogFile(ENGINE* engine) {
  // DOME-2020-02-02-090000.log
  char* filename = "DOME-out.log";
  engine->debug.logFile = fopen(filename, "w+");
}

internal void
ENGINE_printLogVariadic(ENGINE* engine, const char* line, va_list argList) {
  va_list args;
  va_copy(args, argList);
  size_t bufSize = vsnprintf(NULL, 0, line, args) + 1;
  va_end(args);

  char buffer[bufSize];
  buffer[0] = '\0';
  va_copy(args, argList);
  vsnprintf(buffer, bufSize, line, args);
  va_end(args);

  // Output to console
  printf("%s", buffer);

  if (engine->debug.logFile == NULL) {
    ENGINE_openLogFile(engine);
  }
  if (engine->debug.logFile != NULL) {
    // Output to file
    fputs(buffer, engine->debug.logFile);
    fflush(engine->debug.logFile);
  }
}

internal void
ENGINE_printLog(ENGINE* engine, char* line, ...) {
  // Args is mutated by each vsnprintf call,
  // so it needs to be reinitialised.
  va_list args;
  va_start(args, line);
  ENGINE_printLogVariadic(engine, line, args);
  va_end(args);
}

internal ENGINE_WRITE_RESULT
ENGINE_writeFile(ENGINE* engine, const char* path, const char* buffer, size_t length) {
  const char* fullPath;
  if (path[0] != '/') {
    const char* base = BASEPATH_get();
    fullPath = malloc(strlen(base)+strlen(path)+1);
    strcpy((void*)fullPath, base); /* copy name into the new var */
    strcat((void*)fullPath, path); /* add the extension */
  } else {
    fullPath = path;
  }

  ENGINE_printLog(engine, "Writing to filesystem: %s\n", path);
  int result = writeEntireFile(fullPath, buffer, length);
  if (result == ENOENT) {
    result = ENGINE_WRITE_PATH_INVALID;
  } else {
    result = ENGINE_WRITE_SUCCESS;
  }

  if (path[0] != '/') {
    free((void*)fullPath);
  }

  return result;
}

internal char*
ENGINE_readFile(ENGINE* engine, const char* path, size_t* lengthPtr) {
  char pathBuf[PATH_MAX];

  if (strncmp(path, "./", 2) == 0) {
    strcpy(pathBuf, path + 2);
  } else {
    strcpy(pathBuf, path);
  }

  if (engine->tar != NULL) {
    ENGINE_printLog(engine, "Reading from bundle: %s\n", pathBuf);

    char* file = NULL;
    int err = readFileFromTar(engine->tar, pathBuf, lengthPtr, &file);
    if (err == MTAR_ESUCCESS) {
      return file;
    }

    if (DEBUG_MODE) {
      ENGINE_printLog(engine, "Couldn't read %s from bundle: %s. Falling back\n", pathBuf, mtar_strerror(err));
    }
  }

  if (path[0] != '/') {
    strcpy(pathBuf, BASEPATH_get());
    strcat(pathBuf, path);
  }

#ifdef __EMSCRIPTEN__
  emscripten_wget(pathBuf, pathBuf);
#endif
  if (!doesFileExist(pathBuf)) {
    return NULL;
  }

  ENGINE_printLog(engine, "Reading from filesystem: %s\n", pathBuf);
  return readEntireFile(pathBuf, lengthPtr);
}

internal int
ENGINE_taskHandler(ABC_TASK* task) {
  if (task->type == TASK_PRINT) {
    printf("%s\n", (char*)task->data);
    task->resultCode = 0;
    // TODO: Push to SDL Event Queue
  } else if (task->type == TASK_LOAD_FILE) {
    FILESYSTEM_loadEventHandler(task->data);
  } else if (task->type == TASK_WRITE_FILE) {
  }
  return 0;
}

internal bool
ENGINE_setupRenderer(ENGINE* engine, bool vsync) {
  engine->vsyncEnabled = vsync;
  if (engine->renderer != NULL) {
    SDL_DestroyRenderer(engine->renderer);
  }

  int flags = SDL_RENDERER_ACCELERATED;
  if (vsync) {
    flags |= SDL_RENDERER_PRESENTVSYNC;
  }
  engine->renderer = SDL_CreateRenderer(engine->window, -1, flags);
  if (engine->renderer == NULL) {
    return false;
  }
  SDL_RenderSetLogicalSize(engine->renderer, engine->canvas.width, engine->canvas.height);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, engine->canvas.width, engine->canvas.height);
  if (engine->texture == NULL) {
    return false;
  }
  SDL_RenderGetViewport(engine->renderer, &(engine->viewport));
  return true;
}

internal ENGINE*
ENGINE_init(ENGINE* engine) {
  engine->handleText = true;
  engine->fused = false;
  engine->window = NULL;
  engine->renderer = NULL;
  engine->texture = NULL;

  engine->blitBuffer.pixels = calloc(0, 0);
  engine->blitBuffer.width = 0;
  engine->blitBuffer.height = 0;

  engine->lockstep = false;
  engine->debug.avgFps = 58;
  engine->debugEnabled = false;
  engine->debug.alpha = 0.9;

  engine->debug.errorBufMax = 0;
  engine->debug.errorBuf = NULL;
  engine->debug.errorBufLen = 0;
  engine->debug.errorDialog = true;

  // Initialise the canvas offset.
  engine->canvas.pixels = NULL;
  engine->canvas.offsetX = 0;
  engine->canvas.offsetY = 0;
  engine->canvas.width = GAME_WIDTH;
  engine->canvas.height = GAME_HEIGHT;
  DOME_RECT clip = {
    .x = 0,
    .y = 0,
    .w = GAME_WIDTH,
    .h = GAME_HEIGHT
  };
  engine->canvas.clip = clip;

  engine->argv = NULL;
  engine->argc = 0;

  // TODO: handle if we can't allocate memory.
  PLUGIN_COLLECTION_init(engine);

  return engine;
}

internal int
ENGINE_start(ENGINE* engine) {
  int result = EXIT_SUCCESS;
#if defined _WIN32
  SDL_setenv("SDL_AUDIODRIVER", "directsound", true);
#endif
  SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");

  //Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    ENGINE_printLog(engine, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  //Create window
  engine->window = SDL_CreateWindow("DOME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
  if(engine->window == NULL)
  {
    char* message = "Window could not be created! SDL_Error: %s\n";
    ENGINE_printLog(engine, message, SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  ENGINE_setupRenderer(engine, true);
  if (engine->renderer == NULL)
  {
    char* message = "Could not create a renderer: %s";
    ENGINE_printLog(engine, message, SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->canvas.pixels = calloc(engine->canvas.width * engine->canvas.height, sizeof(char) * 4);
  if (engine->canvas.pixels == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->audioEngine = AUDIO_ENGINE_init();
  if (engine->audioEngine == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }
  SDL_StopTextInput();

  ENGINE_EVENT_TYPE = SDL_RegisterEvents(1);

  ABC_FIFO_create(&engine->fifo);
  engine->fifo.taskHandler = ENGINE_taskHandler;

  MAP_init(&engine->moduleMap);

  engine->running = true;

engine_init_end:
  return result;

}

internal void
ENGINE_finishAsync(ENGINE* engine) {
  if (!engine->fifo.shutdown) {
    ABC_FIFO_close(&engine->fifo);
  }
}

internal void
ENGINE_free(ENGINE* engine) {

  if (engine == NULL) {
    return;
  }
  SDL_StopTextInput();


  ENGINE_finishAsync(engine);

  PLUGIN_COLLECTION_free(engine);

  if (engine->audioEngine) {
    AUDIO_ENGINE_free(engine->audioEngine);
    free(engine->audioEngine);
    engine->audioEngine = NULL;
  }

  if (engine->tar != NULL) {
    mtar_close(engine->tar);
  }

  if (engine->moduleMap.head != NULL) {
    MAP_free(&engine->moduleMap);
  }

  if (engine->blitBuffer.pixels != NULL) {
    free(engine->blitBuffer.pixels);
  }

  if (engine->canvas.pixels != NULL) {
    free(engine->canvas.pixels);
  }

  if (engine->texture != NULL) {
    SDL_DestroyTexture(engine->texture);
  }

  if (engine->renderer != NULL) {
    SDL_DestroyRenderer(engine->renderer);
  }

  if (engine->window != NULL) {
    SDL_DestroyWindow(engine->window);
  }

  if (engine->argv != NULL) {
    free(engine->argv[1]);
    free(engine->argv);
  }

  // DEBUG features
  if (engine->debug.logFile != NULL) {
    fclose(engine->debug.logFile);
  }

  if (engine->debug.errorBuf != NULL) {
    free(engine->debug.errorBuf);
  }
}

internal uint32_t
ENGINE_pget(ENGINE* engine, int64_t x, int64_t y) {
  int32_t width = engine->canvas.width;
  int32_t height = engine->canvas.height;
  if (0 <= x && x < width && 0 <= y && y < height) {
    return ((uint32_t*)(engine->canvas.pixels))[width * y + x];
  }
  return 0xFF000000;
}

inline internal void
ENGINE_pset(ENGINE* engine, int64_t x, int64_t y, uint32_t c) {
  CANVAS canvas = engine->canvas;

  // Account for canvas offset
  x += canvas.offsetX;
  y += canvas.offsetY;

  // Draw pixel at (x,y)
  int32_t width = canvas.width;
  DOME_RECT zone = canvas.clip;

  uint8_t newA = ((0xFF000000 & c) >> 24);

  if (newA == 0) {
    return;
  } else if (zone.x <= x && x < zone.x + zone.w && zone.y <= y && y < zone.y + zone.h) {
    if (newA < 0xFF) {
      uint32_t current = ((uint32_t*)(canvas.pixels))[width * y + x];
      double normA = newA / (double)UINT8_MAX;
      double diffA = 1 - normA;

      uint8_t oldR, oldG, oldB, newR, newG, newB;
      getColorComponents(current, &oldR, &oldG, &oldB);
      getColorComponents(c, &newR, &newG, &newB);

      uint8_t a = 0xFF;
      uint8_t r = (diffA * oldR + normA * newR);
      uint8_t g = (diffA * oldG + normA * newG);
      uint8_t b = (diffA * oldB + normA * newB);

      c = (a << 24) | (b << 16) | (g << 8) | r;
    }

    // This is a very hot line, so we use pointer arithmetic for
    // speed!
    *(((uint32_t*)canvas.pixels) + (width * y + x)) = c;
  }
}

internal void
ENGINE_blitBuffer(ENGINE* engine, int32_t x, int32_t y) {
  PIXEL_BUFFER buffer = engine->blitBuffer;

  uint32_t* blitBuffer = buffer.pixels;
  for (size_t j = 0; j < buffer.height; j++) {
    for (size_t i = 0; i < buffer.width; i++) {
      uint32_t c = *(blitBuffer + (j * buffer.width + i));
      ENGINE_pset(engine, x + i, y + j, c);
    }
  }
}

internal uint32_t*
ENGINE_resizeBlitBuffer(ENGINE* engine, size_t width, size_t height) {
  PIXEL_BUFFER* buffer = &engine->blitBuffer;

  size_t oldBufferSize = buffer->width * buffer->height * sizeof(uint32_t);
  size_t newBufferSize = width * height * sizeof(uint32_t);

  if (oldBufferSize < newBufferSize) {
    buffer->pixels = realloc(buffer->pixels, newBufferSize);
    oldBufferSize = newBufferSize;
  }
  memset(buffer->pixels, 0, oldBufferSize);
  buffer->width = width;
  buffer->height = height;
  return buffer->pixels;
}

internal unsigned const char*
defaultFontLookup(utf8_int32_t codepoint) {
  const local_persist unsigned char empty[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
  if (codepoint >= 0 && codepoint < 0x7F) {
    return font8x8_basic[codepoint];
  } else if (codepoint >= 0x80 && codepoint <= 0x9F) {
    codepoint = codepoint - 0x80;
    return font8x8_control[codepoint];
  } else if (codepoint >= 0xA0 && codepoint <= 0xFF) {
    codepoint = codepoint - 0xA0;
    return font8x8_ext_latin[codepoint];
  } else if (codepoint >= 0x390 && codepoint <= 0x3C9) {
    codepoint = codepoint - 0x390;
    return font8x8_greek[codepoint];
  } else if (codepoint >= 0x2500 && codepoint <= 0x257F) {
    codepoint = codepoint - 0x2500;
    return font8x8_box[codepoint];
  } else if (codepoint >= 0x2580 && codepoint <= 0x259F) {
    codepoint = codepoint - 0x2580;
    return font8x8_block[codepoint];
  } else if (codepoint >= 0x3040 && codepoint <= 0x309F) {
    codepoint = codepoint - 0x3040;
    return font8x8_hiragana[codepoint];
  } else if (codepoint >= 0xE541 && codepoint <= 0xE55A) {
    codepoint = codepoint - 0xE541;
    return font8x8_sga[codepoint];
  } else {
    return empty;
  }
}

internal void
ENGINE_print(ENGINE* engine, char* text, int64_t x, int64_t y, uint32_t c) {
  int fontWidth = 8;
  int fontHeight = 8;
  int spacing = (fontHeight / 4);
  int cursor = 0;
  utf8_int32_t codepoint;
  void* v = utf8codepoint(text, &codepoint);
  size_t len = utf8len(text);
  for (size_t pos = 0; pos < len; pos++) {
    if (text[pos] == '\n') {
      cursor = 0;
      y += fontHeight + spacing;
      v = utf8codepoint(v, &codepoint);
      continue;
    }

    uint8_t* glyph = (uint8_t*)defaultFontLookup(codepoint);
    for (int j = 0; j < fontHeight; j++) {
      for (int i = 0; i < fontWidth; i++) {
        uint8_t v = (glyph[j] >> i) & 1;
        if (v != 0) {
          ENGINE_pset(engine, x + cursor + i, y + j, c);
        }
      }
    }
    cursor += fontWidth;
    v = utf8codepoint(v, &codepoint);
  }
}

internal void
blitPixel(void* dest, size_t pitch, int64_t x, int64_t y, uint32_t c) {
  uint32_t* pixel = (uint32_t*)dest + (y * pitch + x);
  *pixel = c;
}

internal void
blitLine(void* dest, size_t destPitch, int64_t x, int64_t y, int64_t w, uint32_t* buf) {
  size_t pitch = destPitch;
  char* pixels = dest;
  int64_t startX = mid(0, x, pitch);
  int64_t endX = mid(0, x + w, pitch);
  size_t lineWidth = endX - startX;
  uint32_t* bufStart = buf + (size_t)fabs(fmin(0, x));
  char* line = pixels + ((y * pitch + startX) * 4);
  memcpy(line, bufStart, lineWidth * 4);
}

internal void
ENGINE_blitLine(ENGINE* engine, int64_t x, int64_t y, int64_t w, uint32_t* buf) {
  CANVAS canvas = engine->canvas;
  DOME_RECT zone = canvas.clip;
  y += canvas.offsetY;
  if (y < max(0, zone.y) || y >= min(canvas.height, zone.y + zone.h)) {
    return;
  }

  int64_t offsetX = canvas.offsetX;
  size_t pitch = canvas.width;

  int64_t screenStart = max(0, zone.x);
  int64_t lineEnd = mid(0, zone.x + zone.w, pitch);
  int64_t screenX = x + offsetX;

  int64_t startX = mid(screenStart, screenX, lineEnd);
  int64_t endX = mid(screenStart, screenX + w, lineEnd);
  int64_t lineWidth = max(0, min(endX, pitch) - startX);

  uint32_t* bufStart = buf;
  char* pixels = canvas.pixels;
  char* line = pixels + ((y * pitch + startX) * 4);
  memcpy(line, bufStart, lineWidth * 4);
}

internal uint32_t*
ENGINE_createMask(ENGINE* engine, uint64_t originalSize, uint32_t color) {
  uint64_t size = originalSize;
  uint32_t* mask = ENGINE_resizeBlitBuffer(engine, size, size);
  for (uint64_t y = 0; y < size; y++) {
    for (uint64_t x = 0; x < size; x++) {
      blitPixel(mask, size, x, y, color);
    }
  }
  return mask;
}

internal void
ENGINE_drawMask(ENGINE* engine, int64_t x, int64_t y) {
  size_t offset = engine->blitBuffer.width / 2;
  ENGINE_blitBuffer(engine, x - offset, y - offset);
}

internal void
ENGINE_line_high(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c) {
  int64_t dx = x2 - x1;
  int64_t dy = y2 - y1;
  int64_t xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  int64_t p = 2 * dx - dy;

  int64_t y = y1;
  int64_t x = x1;
  while(y <= y2) {
    ENGINE_drawMask(engine, x, y);
    if (p > 0) {
      x += xi;
      p = p - 2 * dy;
    }
    p = p + 2 * dx;
    y++;
  }
}

internal void
ENGINE_line_low(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c) {
  int64_t dx = x2 - x1;
  int64_t dy = y2 - y1;
  int64_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  int64_t p = 2 * dy - dx;

  int64_t y = y1;
  int64_t x = x1;
  while(x <= x2) {
    ENGINE_drawMask(engine, x, y);
    if (p > 0) {
      y += yi;
      p = p - 2 * dx;
    }
    p = p + 2 * dy;
    x++;
  }
}

internal void
ENGINE_line(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c, uint64_t size) {
  ENGINE_createMask(engine, size, c);

  if (llabs(y2 - y1) < llabs(x2 - x1)) {
    if (x1 > x2) {
      ENGINE_line_low(engine, x2, y2, x1, y1, c);
    } else {
      ENGINE_line_low(engine, x1, y1, x2, y2, c);
    }
  } else {
    if (y1 > y2) {
      ENGINE_line_high(engine, x2, y2, x1, y1, c);
    } else {
      ENGINE_line_high(engine, x1, y1, x2, y2, c);
    }

  }
}

internal void
ENGINE_circle_filled(ENGINE* engine, int64_t x0, int64_t y0, int64_t r, uint32_t c) {
  int64_t x = 0;
  int64_t y = r;
  int64_t d = round(M_PI - (2*r));


  uint16_t alpha = (0xFF000000 & c) >> 24;
  size_t bufWidth = r * 2 + 1;
  uint32_t buf[bufWidth];
  for (size_t i = 0; i < bufWidth; i++) {
    buf[i] = c;
  }
  if (alpha == 0xFF) {
    while (x <= y) {
      size_t lineWidthX = x * 2 + 1;
      size_t lineWidthY = y * 2 + 1;

      ENGINE_blitLine(engine, x0 - x, y0 + y, lineWidthX, buf);
      ENGINE_blitLine(engine, x0 - x, y0 - y, lineWidthX, buf);

      ENGINE_blitLine(engine, x0 - y, y0 + x, lineWidthY, buf);
      ENGINE_blitLine(engine, x0 - y, y0 - x, lineWidthY, buf);

      if (d < 0) {
        d = d + (M_PI * x) + (M_PI * 2);
      } else {
        d = d + (M_PI * (x - y)) + (M_PI * 3);
        y--;
      }
      x++;
    }
  } else {
    uint32_t* blitBuffer = ENGINE_resizeBlitBuffer(engine, bufWidth, bufWidth);
    size_t pitch = engine->blitBuffer.width;
    while (x <= y) {
      int64_t c = r;
      size_t lineWidthX = x * 2 + 1;
      size_t lineWidthY = y * 2 + 1;
      blitLine(blitBuffer, pitch, c - x, c + y, lineWidthX, buf);
      if (y != 0) {
        blitLine(blitBuffer, pitch, c - x, c - y, lineWidthX, buf);
      }
      blitLine(blitBuffer, pitch, c - y, c + x, lineWidthY, buf);
      if (x != 0) {
        blitLine(blitBuffer, pitch, c - y, c - x, lineWidthY, buf);
      }

      if (d < 0) {
        d = d + (M_PI * x) + (M_PI * 2);
      } else {
        d = d + (M_PI * (x - y)) + (M_PI * 3);
        y--;
      }
      x++;
    }
    ENGINE_blitBuffer(engine, x0 - r, y0 - r);
  }
}

internal void
ENGINE_circle(ENGINE* engine, int64_t x0, int64_t y0, int64_t r, uint32_t c) {
  int64_t x = 0;
  int64_t y = r;
  int64_t d = round(M_PI - (2*r));

  size_t pitch = r * 2 + 1;
  uint32_t* blitBuffer = ENGINE_resizeBlitBuffer(engine, pitch, pitch);
  pitch = engine->blitBuffer.width;

  while (x <= y) {
    blitPixel(blitBuffer, pitch, r + x, r + y , c);
    blitPixel(blitBuffer, pitch, r - x, r - y , c);
    blitPixel(blitBuffer, pitch, r - x, r + y , c);
    blitPixel(blitBuffer, pitch, r + x, r - y , c);
    blitPixel(blitBuffer, pitch, r + y, r + x , c);
    blitPixel(blitBuffer, pitch, r - y, r - x , c);
    blitPixel(blitBuffer, pitch, r - y, r + x , c);
    blitPixel(blitBuffer, pitch, r + y, r - x , c);

    if (d < 0) {
      d = d + (M_PI * x) + (M_PI * 2);
    } else {
      d = d + (M_PI * (x - y)) + (M_PI * 3);
      y--;
    }
    x++;
  }
  ENGINE_blitBuffer(engine, x0 - r, y0 - r);
}

internal inline double
ellipse_getRegion(double x, double y, int32_t rx, int32_t ry) {
  double rxSquare = rx * rx;
  double rySquare = ry * ry;
  return (rySquare*x) / (rxSquare*y);
}

internal void
ENGINE_ellipsefill(ENGINE* engine, int64_t x0, int64_t y0, int64_t x1, int64_t y1, uint32_t c) {

  // Calculate radius
  int64_t swap = x1;
  if (x1 < x0) {
    x1 = x0;
    x0 = swap;
  }
  swap = y1;
  if (y1 < y0) {
    y1 = y0;
    y0 = swap;
  }

  int32_t rx = (x1 - x0) / 2; // Radius on x
  int32_t ry = (y1 - y0) / 2; // Radius on y
  uint32_t rxSquare = rx*rx;
  uint32_t rySquare = ry*ry;
  uint32_t rx2ry2 = rxSquare * rySquare;

  int32_t dx = (rx + 1) * 2;
  int32_t dy = (ry + 1) * 2;

  size_t bufWidth = max(dx, dy);
  uint32_t buf[bufWidth];
  for (size_t i = 0; i < bufWidth; i++) {
    buf[i] = c;
  }
  uint32_t* blitBuffer = ENGINE_resizeBlitBuffer(engine, dx, dy);
  size_t pitch = engine->blitBuffer.width;

  // Start drawing at (0, ry)
  int32_t x = 0;
  int32_t y = ry;
  double d = 0;

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    size_t lineWidthX = x * 2 + 1;
    double xSquare = x*x;
    // evaluate decision parameter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    blitLine(blitBuffer, pitch, rx - x, ry + y, lineWidthX, buf);
    blitLine(blitBuffer, pitch, rx - x, ry - y, lineWidthX, buf);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // evaluate decision parameter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    size_t lineWidthY = x * 2 + 1;
    blitLine(blitBuffer, pitch, rx - x, ry + y, lineWidthY, buf);
    blitLine(blitBuffer, pitch, rx - x, ry - y, lineWidthY, buf);
  };

  ENGINE_blitBuffer(engine, x0, y0);
}

internal void
ENGINE_ellipse(ENGINE* engine, int64_t x0, int64_t y0, int64_t x1, int64_t y1, uint32_t c) {

  int64_t swap = x1;
  if (x1 < x0) {
    x1 = x0;
    x0 = swap;
  }
  swap = y1;
  if (y1 < y0) {
    y1 = y0;
    y0 = swap;
  }

  // Calculate radius
  int32_t rx = llabs(x1 - x0) / 2; // Radius on x
  int32_t ry = llabs(y1 - y0) / 2; // Radius on y
  int32_t rxSquare = rx*rx;
  int32_t rySquare = ry*ry;
  int32_t rx2ry2 = rxSquare * rySquare;

  // Start drawing at (0, ry)
  double x = 0;
  double y = ry;
  double d = 0;

  int32_t width = (rx + 1) * 2;
  int32_t height = (ry + 1) * 2;
  uint32_t* blitBuffer = ENGINE_resizeBlitBuffer(engine, width, height);
  size_t pitch = engine->blitBuffer.width;

  blitPixel(blitBuffer, pitch, rx + x, ry + y , c);
  blitPixel(blitBuffer, pitch, rx + x, ry - y , c);

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    double xSquare = x*x;
    // evaluate decision parameter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    blitPixel(blitBuffer, pitch, rx + x, ry + y , c);
    blitPixel(blitBuffer, pitch, rx - x, ry - y , c);
    blitPixel(blitBuffer, pitch, rx - x, ry + y , c);
    blitPixel(blitBuffer, pitch, rx + x, ry - y , c);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // evaluate decision parameter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    blitPixel(blitBuffer, pitch, rx + x, ry + y , c);
    blitPixel(blitBuffer, pitch, rx - x, ry - y , c);
    blitPixel(blitBuffer, pitch, rx - x, ry + y , c);
    blitPixel(blitBuffer, pitch, rx + x, ry - y , c);
  };
  ENGINE_blitBuffer(engine, x0, y0);
}

internal void
ENGINE_rect(ENGINE* engine, int64_t x, int64_t y, int64_t w, int64_t h, uint32_t c) {
  w = w - 1;
  h = h - 1;
  ENGINE_line(engine, x, y, x, y+h, c, 1);
  ENGINE_line(engine, x, y, x+w, y, c, 1);
  ENGINE_line(engine, x, y+h, x+w, y+h, c, 1);
  ENGINE_line(engine, x+w, y, x+w, y+h, c, 1);
}

internal void
ENGINE_rectfill(ENGINE* engine, int64_t x, int64_t y, int64_t w, int64_t h, uint32_t c) {
  uint16_t alpha = (0xFF000000 & c) >> 24;
  if (alpha == 0x00) {
    return;
  } else {
    int64_t y1 = y;
    int64_t y2 = y + h - 1;

    if (alpha == 0xFF) {
      size_t lineWidth = w; // x2 - x1;
      uint32_t buf[lineWidth + 1];
      for (size_t i = 0; i <= lineWidth; i++) {
        buf[i] = c;
      }
      for (int64_t j = y1; j <= y2; j++) {
        ENGINE_blitLine(engine, x, j, lineWidth, buf);
      }
    } else {
      int64_t x1 = x;
      int64_t x2 = x + w - 1;

      for (int64_t j = y1; j <= y2; j++) {
        for (int64_t i = x1; i <= x2; i++) {
          ENGINE_pset(engine, i, j, c);
        }
      }
    }
  }
}

internal bool
ENGINE_getKeyState(ENGINE* engine, char* keyName) {
  SDL_Keycode keycode =  SDL_GetKeyFromName(keyName);
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
  const uint8_t* state = SDL_GetKeyboardState(NULL);
  return state[scancode];
}

internal int32_t
ENGINE_findMouseCursorIndex(ENGINE* engine, const char* cursorName) {
  for (int index = 0; index < SDL_NUM_SYSTEM_CURSORS; index++) {
    char * name = ENGINE_MOUSE_CURSORS[index];
    if (STRINGS_EQUAL(cursorName, name)) {
      return index;
    }
  }
  return -1;
}

internal void
ENGINE_setMouseCursor(ENGINE* engine, const char* cursorName) {
  SDL_FreeCursor(engine->mouse.cursor);
  int cursorIndex = ENGINE_findMouseCursorIndex(engine, cursorName);
  engine->mouse.cursor = SDL_CreateSystemCursor(cursorIndex);
  SDL_SetCursor(engine->mouse.cursor);
}

internal void
ENGINE_setMouseRelative(ENGINE* engine, bool relative) {
  engine->mouse.relative = relative;
  if (relative) {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GetRelativeMouseState(&(engine->mouse.x), &(engine->mouse.y));
  } else {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_GetMouseState(&(engine->mouse.x), &(engine->mouse.y));
  }
}

internal float
ENGINE_getMouseX(ENGINE* engine) {
  SDL_Rect viewport = engine->viewport;

  int mouseX = engine->mouse.x;
  int winX;
  int winY;
  SDL_GetWindowSize(engine->window, &winX, &winY);
  if (engine->mouse.relative) {
    return mouseX;
  } else {
    return mouseX * fmax((engine->canvas.width / (float)winX), engine->canvas.height / (float)winY) - viewport.x;
  }
}

internal float
ENGINE_getMouseY(ENGINE* engine) {
  SDL_Rect viewport = engine->viewport;

  int mouseY = engine->mouse.y;
  int winX;
  int winY;
  SDL_GetWindowSize(engine->window, &winX, &winY);
  if (engine->mouse.relative) {
    return mouseY;
  } else {
    return mouseY * fmax((engine->canvas.width / (float)winX), engine->canvas.height / (float)winY) - viewport.y;
  }
}

internal bool
ENGINE_getMouseButton(int button) {
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

internal void
ENGINE_updateTextRegion(ENGINE* engine) {
  if (!engine->handleText) {
    return;
  }
  DOME_RECT region = engine->textRegion;
  SDL_Rect viewport = engine->viewport;

  int winX;
  int winY;
  SDL_GetWindowSize(engine->window, &winX, &winY);

  float factor = fmax((engine->canvas.width / (float)winX), engine->canvas.height / (float)winY);
  float nx = (region.x + viewport.x) / factor;
  float ny = (region.y + viewport.y) / factor;

  SDL_Rect rect = {
    .x = nx,
    .y = ny,
    .w = round(region.w / factor),
    .h = round(region.h / factor)
  };

  // TextInputRect is sensitive to current state, so we force a refresh.
  SDL_StopTextInput();
  SDL_StartTextInput();
  SDL_SetTextInputRect(&rect);
}

internal void
ENGINE_drawDebug(ENGINE* engine) {
  char buffer[20];
  ENGINE_DEBUG* debug = &engine->debug;
  // Choose alpha depending on how fast or slow you want old averages to decay.
  // 0.9 is usually a good choice.
  double framesThisSecond = 1000.0 / (debug->elapsed+1);
  double alpha = debug->alpha;
  debug->avgFps = alpha * debug->avgFps + (1.0 - alpha) * framesThisSecond;
  snprintf(buffer, sizeof(buffer), "%.01f FPS", debug->avgFps);   // here 2 means binary
  int32_t width = engine->canvas.width;
  int32_t height = engine->canvas.height;
  int64_t startX = width - 8*8-2;
  int64_t startY = height - 8-2;

  ENGINE_rectfill(engine, startX, startY, 8*8+2, 10, 0x7F000000);
  ENGINE_print(engine, buffer, startX+1,startY+1, 0xFFFFFFFF);

  startX = width - 9*8 - 2;
  if (engine->vsyncEnabled) {
    ENGINE_print(engine, "VSync On", startX, startY - 8, 0xFFFFFFFF);
  } else {
    ENGINE_print(engine, "VSync Off", startX, startY - 8, 0xFFFFFFFF);
  }

  if (engine->lockstep) {
    ENGINE_print(engine, "Lockstep", startX, startY - 16, 0xFFFFFFFF);
  } else {
    ENGINE_print(engine, "Catchup", startX, startY - 16, 0xFFFFFFFF);
  }
}

internal bool
ENGINE_canvasResize(ENGINE* engine, uint32_t newWidth, uint32_t newHeight, uint32_t color) {
  if (engine->canvas.width == newWidth && engine->canvas.height == newHeight) {
    return true;
  }

  engine->canvas.width = newWidth;
  engine->canvas.height = newHeight;
  SDL_DestroyTexture(engine->texture);
  SDL_RenderSetLogicalSize(engine->renderer, newWidth, newHeight);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, newWidth, newHeight);
  if (engine->texture == NULL) {
    return false;
  }

  engine->canvas.pixels = realloc(engine->canvas.pixels, engine->canvas.width * engine->canvas.height * 4);
  if (engine->canvas.pixels == NULL) {
    return false;
  }
  ENGINE_rectfill(engine, 0, 0, engine->canvas.width, engine->canvas.height, color);
  SDL_RenderGetViewport(engine->renderer, &(engine->viewport));
  ENGINE_updateTextRegion(engine);

#ifdef __EMSCRIPTEN__
  SDL_SetWindowSize(engine->window, engine->canvas.width, engine->canvas.height);
#endif

  return true;
}

internal void
ENGINE_takeScreenshot(ENGINE* engine) {
  CANVAS canvas = engine->canvas;
  stbi_write_png("screenshot.png", canvas.width, canvas.height, 4, canvas.pixels, canvas.width * 4);
}


internal void
ENGINE_reportError(ENGINE* engine) {
  if (engine->debug.errorBuf != NULL) {
    ENGINE_printLog(engine, engine->debug.errorBuf);
    if (engine->debug.errorDialog) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             "DOME - Error",
                             engine->debug.errorBuf,
                             NULL);
    }
  }
}
