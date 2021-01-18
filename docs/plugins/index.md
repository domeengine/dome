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


# Plugin Interfaces


## Lifecycle hooks

DOME can call different functions implemented by your plugin, at different times during the game loop.
All hooks return a DOME_Result value, which can be: 
 * `DOME_RESULT_SUCCESS`
 * `DOME_RESULT_FAILURE`
 * `DOME_RESULT_UNKNOWN`

Returning any result other than `DOME_RESULT_SUCCESS` will cause DOME to abort and shutdown. You should use the `dome->log` call to print an error before this.

### Init

```
DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI, DOME_Context ctx)
```

DOME calls this function when the plugin is first loaded, which gives you a chance to perform any initialisation you need to.
You can also signal to DOME that there was a problem by returning the correct error code.

This is also the best opportunity to acquire the available APIs, thanks to the `DOME_getAPI`, which explained in the Callable APIs section.

### Pre-Update
```
DOME_Result PLUGIN_preUpdate(DOME_Context ctx)
```
This hook is called before the Game.update step of the game loop.

### Post-Update
```
DOME_Result PLUGIN_postUpdate(DOME_Context ctx)
```
This hook is called after the Game.update step of the game loop.

### Pre-Draw
```
DOME_Result PLUGIN_preDraw(DOME_Context ctx)
```

This hook is called before the Game.draw step of the game loop.

### Post-Draw
```
DOME_Result PLUGIN_postDraw(DOME_Context ctx)
```
This hook is called after the Game.draw step of the game loop.


### Shutdown
```
DOME_Result PLUGIN_onShutdown(DOME_Context ctx)
```
This hook occurs when the plugin is being unloaded, usually because DOME is in the process of quitting. This is your last opportunity to free any resources your plugin is holding on to, and cleaning up any other background processes.


## Callable APIs

The DOME Plugin API is split into different pieces, divided by general purpose and version. This is to allow maximum compatibility, as new features are added.
The engine  will endeavour to support previous versions of an API for as long as possible, but not guarentees will be made for compatibility across major versions of DOME.

APIs are provided as a struct of function pointers, returned from:
```
void* DOME_getAPI(API_TYPE type, int API_VERSION)
```

### Core

This API allows your plugin to register modules and provides some basic utilities.

#### Acquisition

```
DOME_API_v0* core = (DOME_API_v0*)DOME_getAPI(API_DOME, DOME_API_VERSION);
```

#### Methods

```
DOME_Result registerModule(DOME_Context ctx, const char* name, const char* moduleSource)`
```
This call registers module `name` with the source code `moduleSource`.

```
DOME_Result registerFn(DOME_Context ctx, const char* name, const char* signature, DOME_ForeignFn method)`
```
Register `method` as the function to call for the foreign method specified by `signature` in the module `name`.

The format for the `signature` string is as follows:
 * `static` if the method is a static class method, followed by a space, otherwise both are omitted.
 * `ClassName` for the class method being declared, followed by a period (`.`)
 * `methodName` which is the name of the field/method being exposed.
   - If this is a field getter, nothing else is needed.
   - If this is a field setter, add `=(_)`
   - If this is a method, then parenthesis and a comma seperated list of underscores (`_`) follow, for the number of arguments the method takes. Wren methods can only have up to 16 arguments.

DOME_ForeignFn methods have the signature: `void method(WrenVM* vm)`

```
DOME_Result registerBindFn(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn)
```
Register a method to call when trying to resolve class allocators for the module `moduleName`. A `DOME_BindClassFn` has the signature: `WrenForeignClassMethods method(const char* className)`

```
DOME_Context getContext(WrenVM* vm)
```
This allows foreign functions called by the Wren VM to access the current DOME context, to call various APIs.

```
void log(DOME_Context ctx, const char* text, ...)
```

Using this method allows for formatted output of `text` to the various debugging outputs DOME uses (stdout, a debug console on Windows and a DOME-log.txt file). 

You can use C-style specifiers for the `text` string, as used in the `printf` family of functions.

### Wren

You have access to a subset of the [Wren slot API](https://wren.io/embedding/slots-and-handles.html) in order to access parameters and return values in foreign methods.

#### Acquisition

```
WREN_API_v0* wren = (WREN_API_v0*)DOME_getAPI(API_WREN, WREN_API_VERSION);
```

#### Methods
This is a list of provided methods:
```
      void   ensureSlots(WrenVM* vm, int slotCount);
      void   setSlotNull(WrenVM* vm, int slot);
      void   setSlotBool(WrenVM* vm, int slot, bool value);
      void   setSlotDouble(WrenVM* vm, int slot, double value);
      void   setSlotString(WrenVM* vm, int slot, const char* text);
      void   setSlotBytesWrenVM* vm, int slot, const char* data, size_t length);
      void*  setSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t length);
      bool   getSlotBool(WrenVM* vm, int slot);
      double getSlotDouble(WrenVM* vm, int slot);    
const char*  getSlotString(WrenVM* vm, int slot);   
const char*  getSlotBytes(WrenVM* vm, int slot, int* length);                   
      void   abortFiber(WrenVM* vm, int slot);
```

### Audio

This set of APIs gives you access to DOME's audio engine, to implement your own implementation of audio. You can use this to synthesize sounds, or play your own files.

When you create a new audio channel, you have to supply callbacks for mixing, updating and finalizing the channel. This allows it to play nicely within DOME's expected audio lifecycle.

#### Acquisition

```
AUDIO_API_v0* wren = (AUDIO_API_v0*)DOME_getAPI(API_AUDIO, AUDIO_API_VERSION);
```

#### Methods

```
CHANNEL_REF channelCreate(DOME_Context ctx,
                          CHANNEL_mix mix, 
                          CHANNEL_callback update, 
                          CHANNEL_callback finish, 
                          void* userdata);
```
This creates a channel with the specified callbacks. You can set the `userdata` pointer to any relevant data you like. You are responsible for the management of the memory pointed to by that pointer. This also returns a CHANNEL_REF value, which can be used to manipulate the channel's state during execution.

``` 
void setState(CHANNEL_REF ref, CHANNEL_STATE state)
```
This allows you to specify the channel's status. DOME will only mix in channels in the following states: `CHANNEL_PLAYING`, `CHANNEL_STOPPING` and `CHANNEL_VIRTUALISING`.

``` 
void stop(CHANNEL_REF ref)
```
Marks the audio channel as having stopped. This means that DOME will no longer play this channel. It will call the `finish` callback at it's next opportunity.

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

```


