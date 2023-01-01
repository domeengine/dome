#include <stddef.h>
// You'll need to include the DOME header
#include "dome.h"
#include "libexample.h"

static DOME_API_v0* core;
static WREN_API_v0* wren;

static const char* source =  ""
"foreign class ExternalClass {\n" // Source file for an external module
  "construct init() {} \n"
  "foreign alert(text) \n"
"} \n";

void allocate(WrenVM* vm) {
  size_t CLASS_SIZE = 0; // This should be the size of your object's data
  void* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
}

void alertMethod(WrenVM* vm) {
  // Fetch the method argument
  const char* text = wren->getSlotString(vm, 1);

  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Context ctx = core->getContext(vm);

  core->log(ctx, "%s\n", rust_function());
  core->log(ctx, "%s\n", text);
}

DOME_EXPORT DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {

  // Fetch the latest Core API and save it for later use.
  core = DOME_getAPI(API_DOME, DOME_API_VERSION);

  // DOME also provides a subset of the Wren API for accessing slots
  // in foreign methods.
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  core->log(ctx, "Initialising external module\n");

  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  core->registerModule(ctx, "external", source);

  core->registerClass(ctx, "external", "ExternalClass", allocate, NULL);
  core->registerFn(ctx, "external", "ExternalClass.alert(_)", alertMethod);

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
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

