#include <stddef.h>
// You'll need to include the DOME header
#include "dome.h"

static GRAPHICS_API_v0* graphics;
static DOME_API_v0* core;
static WREN_API_v0* wren;

static const char* source =  ""
"foreign class Raycaster {\n" // Source file for an external module
  "construct init() {} \n"
  "foreign draw(alpha) \n"
"} \n";

void allocate(WrenVM* vm) {
  size_t CLASS_SIZE = 0; // This should be the size of your object's data
  void* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
}

void draw(WrenVM* vm) {
  // Fetch the method argument
  double alpha = wren->getSlotDouble(vm, 1);

  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Context ctx = core->getContext(vm);
  DOME_Color color;
  color.value = 0xFFBBCCDD;

  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 20; x++) {
      graphics->pset(ctx, x, y, color);
    }
  }
}

DOME_EXPORT DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {

  // Fetch the latest Core API and save it for later use.
  core = DOME_getAPI(API_DOME, DOME_API_VERSION);
  graphics = DOME_getAPI(API_GRAPHICS, GRAPHICS_API_VERSION);

  // DOME also provides a subset of the Wren API for accessing slots
  // in foreign methods.
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  core->log(ctx, "Initialising raycaster module\n");

  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  core->registerModule(ctx, "raycaster", source);

  core->registerClass(ctx, "raycaster", "Raycaster", allocate, NULL);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", draw);

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  DOME_Color color = graphics->pget(ctx, 0, 0);
  core->log(ctx, "a: 0x%02hX, r: 0x%02hX, g: 0x%02hX, b: 0x%02hX\n", color.component.a, color.component.r, color.component.g, color.component.b);
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

