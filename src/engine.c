typedef struct {
  SDL_Window* window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  void* pixels;
  ABC_FIFO fifo;
  ForeignFunctionMap fnMap;
  ModuleMap moduleMap;
  uint32_t width;
  uint32_t height;
  mtar_t* tar;
} ENGINE;

typedef enum {
  EVENT_NOP,
  EVENT_LOAD_FILE,
  EVENT_WRITE_FILE,
  EVENT_WRITE_FILE_APPEND
} EVENT_TYPE;

typedef enum {
  TASK_NOP,
  TASK_PRINT,
  TASK_LOAD_FILE,
  TASK_WRITE_FILE,
  TASK_WRITE_FILE_APPEND
} TASK_TYPE;

global_variable uint32_t ENGINE_EVENT_TYPE;

internal char*
ENGINE_readFile(ENGINE* engine, char* path, size_t* lengthPtr) {
  if (engine->tar != NULL) {
    char pathBuf[PATH_MAX];
    strcpy(pathBuf, "\0");
    if (strncmp(path, "./", 2) != 0) {
      strcpy(pathBuf, "./");
    }
    strcat(pathBuf, path);
    printf("Reading tar: %s\n", pathBuf);
    mtar_header_t h;
    int success = mtar_find(engine->tar, pathBuf, &h);
    if (success == MTAR_ESUCCESS) {
      return readFileFromTar(engine->tar, pathBuf, lengthPtr);
    } else if (success != MTAR_ENOTFOUND) {
      printf("Error: There was a problem reading %s from the bundle.\n", pathBuf);
      abort();
    }
    printf("Couldn't find %s in bundle, falling back.\n", pathBuf);
  }

  char* base = SDL_GetBasePath();
  char* fullPath = malloc(strlen(base)+strlen(path)+1);
  strcpy(fullPath, base); /* copy name into the new var */
  strcat(fullPath, path); /* add the extension */
  SDL_free(base);
  if (!doesFileExist(fullPath)) {
    return NULL;
  } else {
    return readEntireFile(fullPath, lengthPtr);
  }
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

internal int
ENGINE_init(ENGINE* engine) {
  int result = EXIT_SUCCESS;
  engine->window = NULL;
  engine->renderer = NULL;
  engine->texture = NULL;
  engine->pixels = NULL;

  //Create window
  engine->window = SDL_CreateWindow("DOME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
  if(engine->window == NULL)
  {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->renderer = SDL_CreateRenderer(engine->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (engine->renderer == NULL)
  {
    SDL_Log("Could not create a renderer: %s", SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }
  SDL_RenderSetLogicalSize(engine->renderer, GAME_WIDTH, GAME_HEIGHT);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GAME_WIDTH, GAME_HEIGHT);
  if (engine->texture == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->pixels = malloc(GAME_WIDTH * GAME_HEIGHT * 4);
  engine->width = GAME_WIDTH;
  engine->height = GAME_HEIGHT;
  if (engine->pixels == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  ENGINE_EVENT_TYPE = SDL_RegisterEvents(1);

  ABC_FIFO_create(&engine->fifo);
  engine->fifo.taskHandler = ENGINE_taskHandler;

  ModuleMap_init(&engine->moduleMap);

engine_init_end:
  return result;
}


internal void
ENGINE_free(ENGINE* engine) {

  if (engine == NULL) {
    return;
  }

  if (engine->tar != NULL) {
    mtar_finalize(engine->tar);
    free(engine->tar);
  }

  ABC_FIFO_close(&engine->fifo);

  if (engine->fnMap.head != NULL) {
    MAP_free(&engine->fnMap);
  }

  if (engine->moduleMap.head != NULL) {
    ModuleMap_free(&engine->moduleMap);
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

internal void
ENGINE_pset(ENGINE* engine, int16_t x, int16_t y, uint32_t c) {
  // Draw pixel at (x,y)
  if ((c & (0xFF << 24)) == 0) {
    return;
  } else if (0 <= x && x < GAME_WIDTH && 0 <= y && y < GAME_HEIGHT) {
    if (((c & (0xFF << 24)) >> 24) < 0xFF) {
      uint32_t current = ((uint32_t*)(engine->pixels))[GAME_WIDTH * y + x];

      // uint16_t oldA = (0xFF000000 & current) >> 24;
      uint16_t newA = (0xFF000000 & c) >> 24;

      uint16_t oldR = (255-newA) * ((0x00FF0000 & current) >> 16);
      uint16_t oldG = (255-newA) * ((0x0000FF00 & current) >> 8);
      uint16_t oldB = (255-newA) * (0x000000FF & current);
      uint16_t newR = newA * ((0x00FF0000 & c) >> 16);
      uint16_t newG = newA * ((0x0000FF00 & c) >> 8);
      uint16_t newB = newA * (0x000000FF & c);
      uint8_t a = newA;
      uint8_t r = (oldR + newR) / 255;
      uint8_t g = (oldG + newG) / 255;
      uint8_t b = (oldB + newB) / 255;

      c = (a << 24) | (r << 16) | (g << 8) | b;
    }
    ((uint32_t*)(engine->pixels))[GAME_WIDTH * y + x] = c;
  }
}

internal void
ENGINE_print(ENGINE* engine, char* text, uint16_t x, uint16_t y, uint32_t c) {
  int fontWidth = 5;
  int fontHeight = 7;
  int cursor = 0;
  for (size_t pos = 0; pos < strlen(text); pos++) {
    char letter = text[pos];

    uint8_t* glyph = (uint8_t*)font[letter - 32];
    if (*glyph == '\n') {
      break;
    }
    for (int j = 0; j < fontHeight; j++) {
      for (int i = 0; i < fontWidth; i++) {
        uint8_t v = glyph[j * fontWidth + i];
        if (v != 0) {
          ENGINE_pset(engine, x + cursor + i, y + j, c);
        }
      }
    }
    cursor += 6;
  }
}

internal void
ENGINE_line_high(ENGINE* engine, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t c) {
  int16_t dx = x2 - x1;
  int16_t dy = y2 - y1;
  int16_t xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  int16_t p = 2 * dx - dy;

  int16_t y = y1;
  int16_t x = x1;
  while(y <= y2) {
    ENGINE_pset(engine, x, y, c);
    if (p > 0) {
      x += xi;
      p = p - 2 * dy;
    } else {
      p = p + 2 * dx;
    }
    y++;
  }
}

internal void
ENGINE_line_low(ENGINE* engine, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t c) {
  int16_t dx = x2 - x1;
  int16_t dy = y2 - y1;
  int16_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  int16_t p = 2 * dy - dx;

  int16_t y = y1;
  int16_t x = x1;
  while(x <= x2) {
    ENGINE_pset(engine, x, y, c);
    if (p > 0) {
      y += yi;
      p = p - 2 * dx;
    } else {
      p = p + 2 * dy;
    }
    x++;
  }
}

internal void
ENGINE_line(ENGINE* engine, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t c) {
  if (abs(y2 - y1) < abs(x2 - x1)) {
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
ENGINE_circle_filled(ENGINE* engine, int16_t x0, int16_t y0, int16_t r, uint32_t c) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = round(M_PI - (2*r));

  while (x <= y) {
    ENGINE_line(engine, x0 - x, y0 + y, x0 + x, y0 + y, c);
    ENGINE_line(engine, x0 - y, y0 + x, x0 + y, y0 + x, c);
    ENGINE_line(engine, x0 + x, y0 - y, x0 - x, y0 - y, c);
    ENGINE_line(engine, x0 - y, y0 - x, x0 + y, y0 - x, c);

    if (d < 0) {
      d = d + (M_PI * x) + (M_PI * 2);
    } else {
      d = d + (M_PI * (x - y)) + (M_PI * 3);
      y--;
    }
    x++;
  }
}

internal void
ENGINE_circle(ENGINE* engine, int16_t x0, int16_t y0, int16_t r, uint32_t c) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = round(M_PI - (2*r));

  while (x <= y) {
    ENGINE_pset(engine, x0 + x, y0 + y, c);
    ENGINE_pset(engine, x0 + y, y0 + x, c);
    ENGINE_pset(engine, x0 - y, y0 + x, c);
    ENGINE_pset(engine, x0 - x, y0 + y, c);

    ENGINE_pset(engine, x0 - x, y0 - y, c);
    ENGINE_pset(engine, x0 - y, y0 - x, c);
    ENGINE_pset(engine, x0 + y, y0 - x, c);
    ENGINE_pset(engine, x0 + x, y0 - y, c);

    if (d < 0) {
      d = d + (M_PI * x) + (M_PI * 2);
    } else {
      d = d + (M_PI * (x - y)) + (M_PI * 3);
      y--;
    }
    x++;
  }
}

internal inline double
ellipse_getRegion(double x, double y, int32_t rx, int32_t ry) {
  double rxSquare = rx * rx;
  double rySquare = ry * ry;
  return (rySquare*x) / (rxSquare*y);
}

internal void
ENGINE_ellipsefill(ENGINE* engine, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t c) {

  // Calculate radius
  int32_t rx = (x1 - x0) / 2; // Radius on x
  int32_t ry = (y1 - y0) / 2; // Radius on y
  uint32_t rxSquare = rx*rx;
  uint32_t rySquare = ry*ry;
  uint32_t rx2ry2 = rxSquare * rySquare;

  // calculate center co-ordinates
  int32_t xc = min(x0, x1) + rx;
  int32_t yc = min(y0, y1) + ry;

  // Start drawing at (0,ry)
  int32_t x = 0;
  int32_t y = ry;
  double d = 0;

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    double xSquare = x*x;
    // valuate decision paramter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    ENGINE_line(engine, xc+x, yc+y, xc-x, yc+y, c);
    ENGINE_line(engine, xc-x, yc-y, xc+x, yc-y, c);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // valuate decision paramter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    ENGINE_line(engine, xc+x, yc+y, xc-x, yc+y, c);
    ENGINE_line(engine, xc-x, yc-y, xc+x, yc-y, c);
  };
}

internal void
ENGINE_ellipse(ENGINE* engine, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t c) {

  // Calcularte radius
  int32_t rx = abs(x1 - x0) / 2; // Radius on x
  int32_t ry = abs(y1 - y0) / 2; // Radius on y
  int32_t rxSquare = rx*rx;
  int32_t rySquare = ry*ry;
  int32_t rx2ry2 = rxSquare * rySquare;

  // calculate center co-ordinates
  int32_t xc = min(x0, x1) + rx;
  int32_t yc = min(y0, y1) + ry;

  // Start drawing at (0,ry)
  double x = 0;
  double y = ry;
  double d = 0;

  ENGINE_pset(engine, xc+x, yc+y, c);
  ENGINE_pset(engine, xc+x, yc-y, c);

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    double xSquare = x*x;
    // valuate decision paramter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    ENGINE_pset(engine, xc+x, yc+y, c);
    ENGINE_pset(engine, xc-x, yc-y, c);
    ENGINE_pset(engine, xc-x, yc+y, c);
    ENGINE_pset(engine, xc+x, yc-y, c);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // valuate decision paramter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    ENGINE_pset(engine, xc+x, yc+y, c);
    ENGINE_pset(engine, xc-x, yc-y, c);
    ENGINE_pset(engine, xc-x, yc+y, c);
    ENGINE_pset(engine, xc+x, yc-y, c);
  };
}

internal void
ENGINE_rect(ENGINE* engine, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t c) {
  ENGINE_line(engine, x, y, x, y+h-1, c);
  ENGINE_line(engine, x, y, x+w-1, y, c);
  ENGINE_line(engine, x, y+h-1, x+w-1, y+h-1, c);
  ENGINE_line(engine, x+w-1, y, x+w-1, y+h-1, c);
}

internal void
ENGINE_rectfill(ENGINE* engine, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t c) {
  int16_t x1 = mid(0, x, GAME_WIDTH);
  int16_t y1 = mid(0, y, GAME_HEIGHT);
  int16_t x2 = mid(0, x + w, GAME_WIDTH);
  int16_t y2 = mid(0, y + h, GAME_HEIGHT);

  for (uint16_t j = y1; j < y2; j++) {
    for (uint16_t i = x1; i < x2; i++) {
      ENGINE_pset(engine, i, j, c);
    }
  }
}

internal bool
ENGINE_getKeyState(ENGINE* engine, char* keyName) {
  SDL_Keycode keycode =  SDL_GetKeyFromName(keyName);
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
  uint8_t* state = SDL_GetKeyboardState(NULL);
  return state[scancode];
}
