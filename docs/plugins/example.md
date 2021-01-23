[< Back](.)

Plugin Example
============
This example has two parts: A plugin file and a DOME application which loads it:

## DOME application

This is a basic example of loading a plugin, and making use of an external module
to access native methods.

```wren

import "plugin" for Plugin
Plugin.load("pluginName")
// The plugin will be initialised now

// Plugins can register their own modules
import "external" for ExternalClass

// and allocators for foreign classes
var obj = ExternalClass.init()

// and finally, they can register foreign methods implemented
// in the plugin native language.
obj.alert("Some words")

```


## Plugin library

A plugin can be written in any language that can compile down to a DLL, .so or .dylib file. This example will use C.

```c

// You'll need to include the DOME header
#include "dome.h"

static DOME_API_v0* core;
static WREN_API_v0* wren;

static const char* source =  ""
"class ExternalClass {\n" // Source file for an external module
  "construct init() {}"
  "foreign alert(text)"
"}";

void allocate(WrenVM* vm) {
  size_t CLASS_SIZE = 0; // This should be the size of your object's data
  void* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
}


WrenForeignClassMethods bindFn(const char* className) {
  WrenForeignClassMethods methods;
  methods.allocate = NULL;
  methods.finalize = NULL;
  if (strcmp(className, "ExternalClass") == 0) {
    methods.allocate = allocate;
  }
  return methods;
}

void alertMethod(WrenVM* vm) {
  // Fetch the method argument
  char* text = wren->getSlotString(1);

  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Context ctx = core->getContext(vm);

  DOME_log(ctx, text);
}

DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
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

  core->registerBindFn(ctx, "external", bindFn);
  core->registerFn(ctx, "external", "ExternalClass.alert(_)", alertMethod);

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

DOME_Result PLUGIN_postUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_Result PLUGIN_preDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_Result PLUGIN_postDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

DOME_Result PLUGIN_onShutdown(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

```

## Compiling your plugin

Compiling plugins is a little tricky, as each platform has slightly different requirements. This section attempts to demonstrate ways of doing this on the common platforms.


The `dome.h` file is located in `../../include`, so we make sure this is added to the include folder list. You can adjust this to suit your development environment as necessary.

### Mac OS X
```
> gcc -dynamiclib -o pluginName.dylib -I../../include plugin.c -undefined dynamic_lookup
```

The resulting file should be in the working directory where DOME is invoked from.

We tell it we are building a `dynamiclib`, and that we want to treat `undefined` functions as fine using `dynamic_lookup`, so that methods can be acquired and linked at runtime. You may also want to add optimisation flags to your compliation command, but each plugin's requirements will be unique.

### Windows
```
> gcc -O3 -std=gnu11 -shared -fPIC  -I../../include pluginName.c -Wl,--unresolved-symbols=ignore-in-object-files -o pluginName.dll
```
This example uses MSYS/MinGW for compiling the library on Windows, but there's no reason you couldn't use Visual Studio to compile a plugin instead. The `.dll` needs to be in the directory where DOME is invoked from.

### Linux:
```
> gcc -O3 -std=c11 -shared -o pluginName.so -fPIC  -I../../include pluginName.c
```
The resulting `.so` file needs be placed somewhere that the operating system knows to look for it. You might need to add the path to `LD_LIBRARY_PATH`, or position it in a user library folder, as appropriate.


