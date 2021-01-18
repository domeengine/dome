[< Back](..)

Plugins
============

Advanced developers can build native plugins, which extend DOME's functions and allow the use of compiled shared libaries. 

This allows for features which require a deeper system access than DOME's module API's expose. For example, you could implement a raw audio synthesizer, or interact with a pre-compiled library such as libCURL to make HTTP calls.

### Caveats 

Using plugins with DOME can hugely expand the functions of your application, but there are certain things to be aware of:

1) Standard DOME applications are safely portable, as the DOME platform is compiled for multiple platforms. This does not extend to plugins, which will need to be compiled for your target platforms and supplied with distributions of your application.
2) DOME cannot verify the correctness of plugin implementations, which means that a badly implemented plugin could cause DOME to crash unexpectedly.
3) Your plugin will need to expose symbols with C-style function names. It cannot access names which have been mangled.




# Plugin API

The DOME Plugin API is split into different pieces, divided by general purpose and version. This is to allow maximum compatibility, as new features are added.
The engine  will endeavour to support previous versions of an API for as long as possible, but not guarentees will be made for compatibility across major versions of DOME.

## Hooks

DOME can call different functions implemented by your plugin, at different times during the game loop.

At minimum, your plugin must provide a function with the following signature:
```
DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI, DOME_Context ctx)
```

DOME calls this function when the plugin is first loaded, which gives you a chance to perform any initialisation you need to.
You can also signal to DOME that there was a problem by returning the correct error code.
This is also the best opportunity to acquire the available APIs, thanks to the `DOME_getAPI` parameter.

 


## Core

## Wren

## Audio








# Example

A basic example requires two parts: A plugin file and a Wren application which loads it:

## Wren application

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

static DOME_API_v0* api;
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
  DOME_Context ctx = api->getContext(vm);

  DOME_log(ctx, text);
}

DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {
  
  // Fetch the latest Core API and save it for later use.
  api = DOME_getAPI(API_DOME, DOME_API_VERSION);

  // DOME also provides a subset of the Wren API for accessing slots
  // in foreign methods.
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  api->log(ctx, "Initialising external module\n");
  
  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  api->registerModule(ctx, "external", source);

  api->registerBindFn(ctx, "external", bindFn);
  api->registerFn(ctx, "external", "ExternalClass.alert(_)", alertMethod);

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

```


