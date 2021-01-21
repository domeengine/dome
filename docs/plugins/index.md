[< Back](..)

Plugins
============

Advanced developers are invited to build native plugins using a compiled language. This allows for deeper system access than DOME's module API's expose, as well as greater performance. It also makes the features of various shared libraries available, at a small cost.

### Caveats 

Using plugins with DOME can hugely expand the functions of your application, but there are certain things to be aware of:

  1. Standard DOME applications are safely portable, as the engine is compiled for multiple platforms. This does not extend to plugins, which will need to be compiled for your target platforms and supplied with distributions of your application.
  2. DOME cannot verify the correctness of plugin implementations, which means that a badly implemented plugin could cause DOME to crash unexpectedly.
  3. Your plugin will need to expose symbols with C-style function names. DOME cannot access functions whose names have been mangled.

# Contents 

 * [Getting Started](#getting-started)
 * Lifecycle hooks
   - [Init](#init)
   - [Pre-Update](#pre-update)
   - [Post-Update](#post-update)
   - [Pre-Draw](#pre-draw)
   - [Post-Draw](#post-draw)
   - [Shutdown](#shutdown)
 * API Services
   - [Core](#core)
     * [enum: DOME_Result](#enum-dome_result)
     * [method: registerModule](#method-registermodule)
     * [method: registerFn](#method-registerfn)
     * [method: registerBindFn](#method-registerbindfn)
     * [method: getContext](#method-getcontext)
     * [method: log](#method-log)
   - [Wren](#wren)
   - [Audio](#audio)
     * [method: channelCreate](#method-channelcreate)
     * [method: getData](#method-getdata)
     * [method: getState](#method-getstate)
     * [method: setState](#method-setstate)
     * [method: stop](#method-stop)
     * [enum: CHANNEL_STATE](#enum-channel_state)
 * [Example](#example)


# Getting Started
In order to start writing your plugins, you will need to include `dome.h` in your project. This file can be found [here](https://github.com/domeengine/dome/blob/main/includes/dome.h) in the `includes` folder of the GitHub repository.
You will also need to configure your compiler/linker to ignore undefined methods and output a shared library. DOME supports plugins compiled as `.dll` (on Windows), `.so` (on Linux) and `.dylib` (on Mac OS X).

The compiled library has to be available in the shared library path with respect to the working directory DOME is invoked from, not necessarily where DOME is located on disk, or where your application's `main.wren` or `game.egg` file is.

# Plugin Interfaces

## Lifecycle hooks

DOME can call specially named functions implemented by your plugin, at different times during the game loop. For this to work, you must ensure that your compiler does not mangle names.

Returning any result other than `DOME_RESULT_SUCCESS` will cause DOME to abort and shutdown. You should use the [`log(text)`](#method-log) call to print an error before this.

### Init

```
DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI, DOME_Context ctx)
```

DOME calls this function when the plugin is first loaded, which gives you a chance to perform any initialisation you need to.
You can also signal to DOME that there was a problem by returning the correct error code.

This is also the best opportunity to acquire the available APIs, thanks to the `DOME_getAPI` function pointer, which is explained in the [API Services](#api-services) section. The structs returned from this call should be stored for use throughout the lifetime of your plugin.

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



# API Services

The DOME Plugin API is split into different pieces, divided by general purpose and version. This is to allow maximum backwards-compatibility, as new features are added.
The engine  will endeavour to support previous versions of an API for as long as possible, but no guarentees will be made for compatibility across major versions of DOME.

APIs are provided as a struct of function pointers, returned from:
```
void* DOME_getAPI(API_TYPE type, int API_VERSION)
```

## Core

This API allows your plugin to register modules and provides some basic utilities.

### Acquisition

```
DOME_API_v0* core = (DOME_API_v0*)DOME_getAPI(API_DOME, DOME_API_VERSION);
```

### Enums: 

#### enum: DOME_Result

Various methods return an enum of type `DOME_Result`, which indicates success or failure. These are the valid values:

 * `DOME_RESULT_SUCCESS`
 * `DOME_RESULT_FAILURE`
 * `DOME_RESULT_UNKNOWN`


### Methods

#### method: registerModule
```
DOME_Result registerModule(DOME_Context ctx, const char* name, const char* moduleSource)`
```
This call registers module `name` with the source code `moduleSource`. You cannot register modules with the same name as DOME's internal modules. These are reserved.

#### method: registerFn
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

#### method: registerBindFn
```
DOME_Result registerBindFn(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn)
```
Register a method to call when trying to resolve class allocators for the module `moduleName`. A `DOME_BindClassFn` has the signature: `WrenForeignClassMethods method(const char* className)`


#### method: getContext
```
DOME_Context getContext(WrenVM* vm)
```
This allows foreign functions called by the Wren VM to access the current DOME context, to call various APIs.

#### method: log
```
void log(DOME_Context ctx, const char* text, ...)
```

Using this method allows for formatted output of `text` to the various debugging outputs DOME uses (stdout, a debug console on Windows and a DOME-log.txt file). 

You can use C-style specifiers for the `text` string, as used in the `printf` family of functions.

## Wren

You have access to a subset of the [Wren slot API](https://wren.io/embedding/slots-and-handles.html) in order to access parameters and return values in foreign methods.

### Acquisition

```
WREN_API_v0* wren = (WREN_API_v0*)DOME_getAPI(API_WREN, WREN_API_VERSION);
```

### Methods
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

## Audio

This set of APIs gives you access to DOME's audio engine, to implement your own implementation of audio. You can use this to synthesize sounds, or play your own files.

When you create a new audio channel, you have to supply callbacks for mixing, updating and finalizing the channel. This allows it to play nicely within DOME's expected audio lifecycle.

### Acquisition

```
AUDIO_API_v0* wren = (AUDIO_API_v0*)DOME_getAPI(API_AUDIO, AUDIO_API_VERSION);
```

### Methods

#### method: channelCreate
```
CHANNEL_REF channelCreate(DOME_Context ctx,
                          CHANNEL_mix mix, 
                          CHANNEL_callback update, 
                          CHANNEL_callback finish, 
                          void* userdata);
```
This creates a channel with the specified callbacks. You can set the `userdata` pointer to any relevant data you like. You are responsible for the management of the memory pointed to by that pointer. This also returns a CHANNEL_REF value, which can be used to manipulate the channel's state during execution.

* `CHANNEL_mix` functions have a signature of `void mix(CHANNEL_REF ref, float* buffer, size_t sampleRequestSize)`. `ref` is a reference to the channel being mixed. `buffer` is an interleaved stereo buffer to write your audio data into. One sample is two values, for left and right, so `buffer` is `2 * sampleRequestSize` in size. 
* `CHANNEL_callback` functions have this signature: `void callback(CHANNEL_REF ref, WrenVM* vm)`.
  + `update` is called once a frame, and can be used for safely modifying the state of the channel data.
  + `finish` is called once the channel as been set to `STOPPED`, before its memory is released.
* `userdata` is a pointer set by the plugin developer, which can be used to pass through associated data, and retrieved by [`getData(ref)`](#method-getdata)

#### method: getData
``` 
void* getData(CHANNEL_REF ref)
```
Fetch the `userdata` pointer for the given channel `ref`.

#### method: getState
``` 
CHANNEL_STATE getState(CHANNEL_REF ref)
```
Get the current [state](#enum-channel_state) of the channel specified by `ref`.

#### method: setState
``` 
void setState(CHANNEL_REF ref, CHANNEL_STATE state)
```
This allows you to specify the channel's [state](#enum-channel_state). DOME will only mix in channels in the following states: `CHANNEL_PLAYING` and `CHANNEL_STOPPING`.

#### method: stop
``` 
void stop(CHANNEL_REF ref)
```
Marks the audio channel as having stopped. This means that DOME will no longer play this channel. It will call the `finish` callback at it's next opportunity.
 
#### enum: CHANNEL_STATE

Audio channels are enabled and disabled based on a state, which is represented by this enum. Supported states are the following:

```
enum CHANNEL_STATE {
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED
}
```

# Example

You can find a well-commented example plugin on [this](example) page, which demonstrates all the currently available lifecycle hooks.
