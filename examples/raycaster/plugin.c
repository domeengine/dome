#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "sbuf.c"

#include "domath.c"
#include "dome.h"
#include "renderer.h"
#include "gfx.c"
#include "renderer.c"
#include "object.c"
#include "tile.c"

DOME_EXPORT DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {

  // Fetch the latest Core API and save it for later use.
  core = DOME_getAPI(API_DOME, DOME_API_VERSION);
  io = DOME_getAPI(API_IO, IO_API_VERSION);
  canvas = DOME_getAPI(API_CANVAS, CANVAS_API_VERSION);
  bitmap = DOME_getAPI(API_BITMAP, BITMAP_API_VERSION);
  unsafePset = canvas->unsafePset;
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);
  vm = core->getVM(ctx);
  core->log(ctx, "Initialising raycaster module\n");

  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  core->registerModule(ctx, "raycaster", rendererModuleSource);

  RENDERER_register(ctx);
  TILE_register(ctx);
  OBJ_register(ctx);

  core->lockModule(ctx, "raycaster");

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  // core->log(ctx, "a: 0x%02hX, r: 0x%02hX, g: 0x%02hX, b: 0x%02hX\n", color.component.a, color.component.r, color.component.g, color.component.b);
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_postUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_EXPORT DOME_Result PLUGIN_preDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_EXPORT DOME_Result PLUGIN_postDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_onShutdown(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

