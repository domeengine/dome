[< Back](..)

Native Plugins
============


Advanced developers are invited to build native plugins using a compiled language like C, C++ and Rust. This allows for deeper system access than DOME's module API's expose, as well as greater performance. It also makes the features of various shared libraries available, at a small cost.

> Caution: The Native Plugin API is in an experimental phase and we may need to make some breaking changes in the future. Please keep an eye on this page for updates.

# Contents 

 * [Getting Started](#getting-started)
   - [Example](#example)
   - [Caveats](#caveats)
 * Lifecycle hooks
   - [Init](#init)
   - [Pre-Update](#pre-update)
   - [Post-Update](#post-update)
   - [Pre-Draw](#pre-draw)
   - [Post-Draw](#post-draw)
   - [Shutdown](#shutdown)
 * API Services
   - [Core](#core)
     * Enums
       - [enum: DOME_Result](#enum-dome_result)
     * Function Signatures
       - [function: DOME_ForeignFn](#function-dome_foreignfn)
       - [function: DOME_FinalizerFn](#function-dome_finalizerfn)
     * Methods
       - [method: registerModule](#method-registermodule)
       - [method: registerClass](#method-registerclass)
       - [method: registerFn](#method-registerfn)
       - [method: lockModule](#method-lockmodule)
       - [method: getContext](#method-getcontext)
       - [method: log](#method-log)
   - [Wren](#wren)
   - [Audio](#audio)
     * Enums
       - [enum: CHANNEL_STATE](#enum-channel_state)
     * Function Signatures
       - [function: CHANNEL_mix](#function-channel_mix)
       - [function: CHANNEL_callback](#function-channel_callback)
     * Methods
       - [method: channelCreate](#method-channelcreate)
       - [method: getData](#method-getdata)
       - [method: getState](#method-getstate)
       - [method: setState](#method-setstate)
       - [method: stop](#method-stop)


# Getting Started
In order to start writing your plugins, you will need to include `dome.h` in your project. This file can be found [here](https://github.com/domeengine/dome/blob/main/includes/dome.h) in the `includes` folder of the GitHub repository.
You will also need to configure your compiler/linker to ignore undefined methods and output a shared library. DOME supports plugins compiled as `.dll` (on Windows), `.so` (on Linux) and `.dylib` (on Mac OS X).

The compiled library has to be available in the shared library path, which varies by operating system convention, however usually it can be placed in the same folder as your application entry point. You should consult your operating system's developer documentation for more details.

You can load the plugin from your DOME application by calling [`Plugin.load(name)`](/modules/plugin)

## Example

You can find a well-commented example plugin and application on [this](example) page, which demonstrates all the currently available lifecycle hooks.

## Caveats 

Using plugins with DOME can hugely expand the functions of your application, but there are certain things to be aware of:

  1. Standard DOME applications are safely portable, as the engine is compiled for multiple platforms. This does not extend to plugins, which will need to be compiled for your target platforms and supplied with distributions of your application.
  2. DOME cannot verify the correctness of plugin implementations, which means that a plugin which has bugs could cause DOME to crash unexpectedly, or cause other issues with the underlying system.
  3. Your plugin will need to expose symbols with C-style function names. DOME cannot access functions whose names have been mangled.

# Plugin Interfaces

## Lifecycle hooks

DOME can call specially named functions implemented by your plugin, at different times during the game loop. For this to work, you must ensure that your compiler does not mangle names.

If a hook returns any result other than `DOME_RESULT_SUCCESS`, DOME will abort and shutdown. You should use the [`log(text)`](#method-log) call to print an error before returning.

### Init

```c
DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
                          DOME_Context ctx)
```

DOME calls this function when the plugin is loaded, which gives you a chance to perform any initialisation you need to.
You can also signal to DOME that there was a problem by returning `DOME_RESULT_FAILURE`.

This is also the best opportunity to acquire the available APIs, thanks to the `DOME_getAPI` function pointer, which is explained in the [API Services](#api-services) section. The structs returned from this call should be stored for use throughout the lifetime of your plugin.

### Pre-Update
```c
DOME_Result PLUGIN_preUpdate(DOME_Context ctx)
```
This hook is called before the Game.update step of the game loop.

### Post-Update
```c
DOME_Result PLUGIN_postUpdate(DOME_Context ctx)
```
This hook is called after the Game.update step of the game loop.

### Pre-Draw
```c
DOME_Result PLUGIN_preDraw(DOME_Context ctx)
```

This hook is called before the Game.draw step of the game loop.

### Post-Draw
```c
DOME_Result PLUGIN_postDraw(DOME_Context ctx)
```
This hook is called after the Game.draw step of the game loop.


### Shutdown
```c
DOME_Result PLUGIN_onShutdown(DOME_Context ctx)
```
This hook occurs when the plugin is being unloaded, usually because DOME is in the process of quitting. This is your last opportunity to free any resources your plugin is holding on to, and cleaning up any other background processes.



# API Services

The DOME Plugin API is split into different pieces, divided by general purpose and version. This is to allow maximum backwards-compatibility as new features are added.
The engine will endeavour to support previous versions of an API for as long as possible, but no guarentees will be made for compatibility across major versions of DOME.

APIs are provided as a struct of function pointers, returned from:
```c
void* DOME_getAPI(API_TYPE type, int API_VERSION)
```

## Core

This API allows your plugin to register modules and provides some basic utilities.

### Acquisition

```c
DOME_API_v0* core = (DOME_API_v0*)DOME_getAPI(API_DOME, DOME_API_VERSION);
```

### Enums: 

#### enum: DOME_Result

Various methods return an enum of type `DOME_Result`, which indicates success or failure. These are the valid values:

 * `DOME_RESULT_SUCCESS`
 * `DOME_RESULT_FAILURE`
 * `DOME_RESULT_UNKNOWN`

### Function signatures

#### function: DOME_ForeignFn
`DOME_ForeignFn` methods have the signature: `void method(WrenVM* vm)` to match the `WrenForeignMethodFn` type.

#### function: DOME_FinalizerFn
`DOME_FinalizerFn` methods have the signature: `void finalize(void* vm)`, to match the `WrenFinalizerFn` type.

### Methods

#### method: registerModule
```c
DOME_Result registerModule(DOME_Context ctx, 
                           const char* name, 
                           const char* moduleSource)
```
This call registers module `name` with the source code `moduleSource`. You cannot register modules with the same name as DOME's internal modules. These are reserved.
Returns `DOME_RESULT_SUCCESS` if the module was successfully registered, and `DOME_RESULT_FAILURE` otherwise.

#### method: registerClass
```c
DOME_Result registerClass(DOME_Context ctx, 
                          const char* moduleName, 
                          const char* className, 
                          DOME_ForeignFn allocate, 
                          DOME_FinalizerFn finalize)
```
Register the `allocate` and `finalize` methods for `className` in `moduleName`, so that instances of the foreign class can be allocated, and optionally finalized.
The `finalize` method is your final chance to deal with the userdata attached to your foreign class. You won't have VM access inside this method.

Returns `DOME_RESULT_SUCCESS` if the class is registered and `DOME_RESULT_FAILURE` otherwise. Failure will occur if `allocate` method is provided. The `finalize` argument can optionally be `NULL`.


#### method: registerFn
```c
DOME_Result registerFn(DOME_Context ctx, 
                       const char* name, 
                       const char* signature, 
                       DOME_ForeignFn method)
```
Register `method` as the function to call for the foreign method specified by `signature` in the module `name`. Returns `DOME_RESULT_SUCCESS` if the function was successfully registered, and `DOME_RESULT_FAILURE` otherwise.

The format for the `signature` string is as follows:
 * `static` if the method is a static class method, followed by a space, otherwise both are omitted.
 * `ClassName` for the class method being declared, followed by a period (`.`)
 * `methodName` which is the name of the field/method being exposed.
   - If this is a field getter, nothing else is needed.
   - If this is a field setter, add `=(_)`
   - If this is a method, then parenthesis and a comma seperated list of underscores (`_`) follow, for the number of arguments the method takes. 
   - You can also use the setter and getter syntax for the class' subscript operator `[]`, which can be defined with one or more parameters.
   - Wren methods can have up to 16 arguments, and are overloaded by arity. For example, `Test.do(_)` is considered different to `Test.do(_,_)` and so on.
   
#### method: lockModule
```c
void lockModule(DOME_Context ctx, const char* name)
```
This marks the module `name` as locked, so that further functions cannot modify it. It is recommended to do this after you have registered all the methods for your module, however there is no requirement to.




#### method: getContext
```c
DOME_Context getContext(WrenVM* vm)
```
This allows foreign functions called by the Wren VM to access the current DOME context, to call various APIs.

#### method: log
```c
void log(DOME_Context ctx, const char* text, ...)
```

Using this method allows for formatted output of `text` to the various debugging outputs DOME uses (stdout, a debug console on Windows and a DOME-log.txt file). 

You can use C-style specifiers for the `text` string, as used in the `printf` family of functions.

## Wren

You have access to a subset of the [Wren slot API](https://wren.io/embedding/slots-and-handles.html) in order to access parameters and return values in foreign methods.
The methods are incredibly well documented in the [Wren public header](https://github.com/wren-lang/wren/blob/main/src/include/wren.h), so we will not be documenting the functions here.
 
You do not need to include the `wren.h` header in your application, as `dome.h` includes everything you need.

### Acquisition

```c
WREN_API_v0* wren = (WREN_API_v0*)DOME_getAPI(API_WREN, WREN_API_VERSION);
```

### Methods
This is a list of provided methods:
```c
      void     ensureSlots(WrenVM* vm, int slotCount);
      void     setSlotNull(WrenVM* vm, int slot);
      void     setSlotBool(WrenVM* vm, int slot, bool value);
      void     setSlotDouble(WrenVM* vm, int slot, double value);
      void     setSlotString(WrenVM* vm, int slot, const char* text);
      void     setSlotBytesWrenVM* vm, int slot, const char* data, size_t length);
      void*    setSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t length);
      bool     getSlotBool(WrenVM* vm, int slot);
      double   getSlotDouble(WrenVM* vm, int slot);    
const char*    getSlotString(WrenVM* vm, int slot);   
const char*    getSlotBytes(WrenVM* vm, int slot, int* length);                   
      void     abortFiber(WrenVM* vm, int slot);

      WrenType getSlotType(WrenVM* vm, int slot);

      int      getListCount(WrenVM* vm, int slot);
      void     getListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
      void     setListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
      void     insertInList(WrenVM* vm, int listSlot, int index, int elementSlot);

      int      getMapCount(WrenVM* vm, int slot);
      bool     getMapContainsKey(WrenVM* vm, int mapSlot, int keySlot);
      void     getMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
      void     setMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
      void     removeMapValue(WrenVM* vm, int mapSlot, int keySlot, int removedValueSlot);
```

### Embed

If your plugin requires using _Wren_ files, you can _embed_ them using the built in command `--embed`.
This command converts a _Wren_ source file to a _C_ include file.

```sh
$ dome -e | --embed [--] sourceFile [moduleName] [destinationFile]
```

Example:

```sh
$ dome -e external.wren source external.wren.inc
```


## Audio

This set of APIs gives you access to DOME's audio engine, to provide your own audio channel implementations. You can use this to synthesize sounds, or play custom audio formats.

### Acquisition

```c
AUDIO_API_v0* wren = (AUDIO_API_v0*)DOME_getAPI(API_AUDIO, AUDIO_API_VERSION);
```

### Enums

#### enum: CHANNEL_STATE

Audio channels are enabled and disabled based on a state, which is represented by this enum. Supported states are the following:

```c
enum CHANNEL_STATE {
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED
}
```

### Function Signatures

#### function: CHANNEL_mix
`CHANNEL_mix` functions have a signature of `void mix(CHANNEL_REF ref, float* buffer, size_t sampleRequestSize)`. 

  * `ref` is a reference to the channel being mixed. 
  * `buffer` is an interleaved stereo buffer to write your audio data into. One sample is two values, for left and right, so `buffer` is `2 * sampleRequestSize` in size. 

#### function: CHANNEL_callback
`CHANNEL_callback` functions have this signature: `void callback(CHANNEL_REF ref, WrenVM* vm)`.


### Methods

#### method: channelCreate
```c
CHANNEL_REF channelCreate(DOME_Context ctx,
                          CHANNEL_mix mix, 
                          CHANNEL_callback update, 
                          CHANNEL_callback finish, 
                          void* userdata);
```

When you create a new audio channel, you must supply callbacks for mixing, updating and finalizing the channel. This allows it to play nicely within DOME's expected audio lifecycle.

This method creates a channel with the specified callbacks and returns its corresponding CHANNEL_REF value, which can be used to manipulate the channel's state during execution. The channel will be created in the state `CHANNEL_INITIALIZE`, which gives you the opportunity to set up the channel configuration before it is played.

The callbacks work like this:
  - `update` is called once a frame, and can be used for safely modifying the state of the channel data.
  - `finish` is called once the channel has been set to `STOPPED`, before its memory is released.

The `userdata` is a pointer set by the plugin developer, which can be used to pass through associated data, and retrieved by [`getData(ref)`](#method-getdata). You are responsible for the management of the memory pointed to by that pointer. 


#### method: getData
```c
void* getData(CHANNEL_REF ref)
```
Fetch the `userdata` pointer for the given channel `ref`.

#### method: getState
```c
CHANNEL_STATE getState(CHANNEL_REF ref)
```
Get the current [state](#enum-channel_state) of the channel specified by `ref`.

#### method: setState
```c 
void setState(CHANNEL_REF ref, CHANNEL_STATE state)
```
This allows you to specify the channel's [state](#enum-channel_state). DOME will only mix in channels in the following states: `CHANNEL_PLAYING` and `CHANNEL_STOPPING`.

#### method: stop
```c
void stop(CHANNEL_REF ref)
```
Marks the audio channel as having stopped. This means that DOME will no longer play this channel. It will call the `finish` callback at it's next opportunity.
