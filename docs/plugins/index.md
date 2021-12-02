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
     * [Module Embedding](#module-embedding)
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
   - [Bitmap](#bitmap)
     * Struct
       - [struct: DOME_Bitmap](#method-dome_bitmap)
     * Methods
       - [method: fromFile](#method-fromfile)
       - [method: fromBuffer](#method-fromfile)
       - [method: free](#method-free)
       - [method: pget](#method-pget)
       - [method: pset](#method-pset)
    - [Canvas](#canvas)
     * Enums
       - [enum: DOME_DrawMode](#enum-dome_drawmode)
     * Struct
       - [struct: DOME_Color](#method-dome_color)
     * Methods
       - [method: draw](#method-draw)
       - [method: getWidth](#method-getwidth)
       - [method: getHeight](#method-getHeight)
       - [method: line](#method-line)
       - [method: pget](#method-pget)
       - [method: pset](#method-pset)
       - [method: unsafePset](#method-unsafepset)
    - [I/O](#io)
     * Methods
       - [method: readfile](#method-readfile)



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
DOME creates a copy of the `name` and `moduleSource`, so you are able to free the pointers if necessary.
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
DOME creates a copy of the `className`, so you are able to free the pointer if necessary.

Returns `DOME_RESULT_SUCCESS` if the class is registered and `DOME_RESULT_FAILURE` otherwise. Failure will occur if `allocate` method is provided. The `finalize` argument can optionally be `NULL`.


#### method: registerFn
```c
DOME_Result registerFn(DOME_Context ctx, 
                       const char* name, 
                       const char* signature, 
                       DOME_ForeignFn method)
```
Register `method` as the function to call for the foreign method specified by `signature` in the module `name`. 
DOME creates a copy of the `signature`, so you are able to free the pointer if necessary.
Returns `DOME_RESULT_SUCCESS` if the function was successfully registered, and `DOME_RESULT_FAILURE` otherwise.

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
      void*    getSlotForeign(WrenVM* vm, int slot);                   

      WrenType getSlotType(WrenVM* vm, int slot);

      void     setSlotNewList(WrenVM* vm, int slot);
      int      getListCount(WrenVM* vm, int slot);
      void     getListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
      void     setListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
      void     insertInList(WrenVM* vm, int listSlot, int index, int elementSlot);

      void     setSlotNewMap(WrenVM* vm, int slot);
      int      getMapCount(WrenVM* vm, int slot);
      bool     getMapContainsKey(WrenVM* vm, int mapSlot, int keySlot);
      void     getMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
      void     setMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
      void     removeMapValue(WrenVM* vm, int mapSlot, int keySlot, int removedValueSlot);


WrenInterpretResult interpret(WrenVM* vm, const char* module, const char* source);
WrenInterpretResult call(WrenVM* vm, WrenHandle* method);

      bool     hasModule(WrenVM* vm, const char* module);
      bool     hasVariable(WrenVM* vm, const char* module, const char* name);
      void     getVariable(WrenVM* vm, const char* module, const char* name, int slot);
   WrenHandle* getSlotHandle(WrenVM* vm, int slot);
      void     setSlotHandle(WrenVM* vm, int slot, WrenHandle* handle);
      void     releaseHandle(WrenVM* vm, WrenHandle* handle);
      void     abortFiber(WrenVM* vm, int slot);
```

### Module Embedding

If your plugin registers a Wren module, you can embed the source of that module in your plugin by using DOME's built-in `embed` subcommand, which will convert it into a C include file.

```sh
$ dome embed sourceFile [moduleVariableName] [destinationFile]
```

Example:

```sh
$ dome embed external.wren sourceModule external.wren.inc
```

This command will use `external.wren` to generate `external.wren.inc`, which contains the variable `sourceModule` for including in C/C++ source code.

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

This callback is called on DOME's Audio Engine mixer thread. It is essential that you avoid any slow operations (memory allocation, network) or you risk interruptions to the audio playback.

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
  - `update` is called once a frame, and can be used for safely modifying the state of the channel data. This callback holds a lock over the mixer thread, so avoid holding it for too long.
  - `finish` is called once the channel has been set to `STOPPED`, before its memory is released. It is safe to expect that the channel will not be played again.

The `userdata` is a pointer set by the plugin developer, which can be used to pass through associated data, and retrieved by [`getData(ref)`](#method-getdata). You are responsible for the management of the memory pointed to by that pointer and should avoid modifying the contents of the memory outside of the provided callbacks.


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

## Bitmap

This set of APIs allows you to load and manage graphics from supported file formats. (See the graphics module for more information.)

### Acquisition

```c
BITMAP_API_v0* bitmap = (BITMAP_API_v0*)DOME_getAPI(API_BITMAP, BITMAP_API_VERSION);
```

### Struct

#### DOME_Bitmap

The DOME_Bitmap type contains the following fields:

| Field  | Type        | Purpose                                   |
| ---------------------------------------------------------------- |
| width  | int32_t     | Width of the bitmap in pixels.            |
| height | int32_t     | Height of the bitmap in pixels.           |
| pixels | DOME_Color* | Pointer to the first pixel of the bitmap. |

### Methods

#### method: fromFile
```c
DOME_Bitmap* fromFile(DOME_Context ctx, const char* path)
```
Loads an image file from `path` on disk (relative to the application entry point)
and returns a `DOME_Bitmap`. You are responsible for freeing it using the `free`
function in this API.

##### method: fromFileInMemory
```c
DOME_Bitmap* fromFileInMemory(DOME_Context ctx, void* buffer, size_t length)
```
Loads an image file stored in memory at `buffer`, with a size of `length`, and
returns a `DOME_Bitmap`. You are responsible for freeing it using the `free`
function in this API.

##### method: free
```c
void free(DOME_Bitmap* bitmap)
```
Safely frees the `bitmap`. Make sure you don't attempt to use the `bitmap` pointer
after this function returns.

##### method: pget
```c
DOME_Color pget(DOME_Bitmap* bitmap, uint32_t x, uint32_t y)
```
Returns the color of the pixel located at `(x, y)` in `bitmap`.

##### method: pset
```c
void pset(DOME_Bitmap* bitmap, uint32_t x, uint32_t y, DOME_Color color)
```
Sets the color of the pixel located at `(x, y)` in `bitmap` to `color`.

## Canvas

This set of APIs allows you to modify what is displayed on the main canvas.
You can exploit this to allow for more efficient graphical rendering techniques.

### Acquisition

```c
CANVAS_API_v0* canvas = (CANVAS_API_v0*)DOME_getAPI(API_CANVAS, CANVAS_API_VERSION);
```

### Enums
#### enum: DOME_DrawMode

Some methods in this API allow you to enable or disable alpha-blending 
for performance gains.

```c
enum DOME_DrawMode {
  DOME_DRAWMODE_BLEND
}
```

### Struct
#### struct: DOME_Color

The DOME_Bitmap type contains the following fields:

| Field  | Type        | Purpose                                          |
| ----------------------------------------------------------------------- |
|    a   | uint8_t     | This is the color's alpha channel, from 0 - 255. |
|    r   | uint8_t     | This is the color's red channel, from 0 - 255.   |
|    g   | uint8_t     | This is the color's green channel, from 0 - 255. |
|    b   | uint8_t     | This is the color's blue channel, from 0 - 255.  |

This type is also a union. You can get all the fields simultaneously as a 
32-bit integer, `value`, arranged in the layout `0xAARRGGBB`.

#### Methods
##### method: draw
```c
void draw(DOME_Context ctx, DOME_Bitmap* bitmap, int32_t x, int32_t y, DOME_DRAWODE mode)
```
Draws the `bitmap` to the canvas at `(x, y)`. If the `mode` is set, alpha-blending
will be applied. This will ignore the canvas draw context (offset, clipping region, etc).

##### method: getWidth
```c
uint32_t getWidth(DOME_Context ctx)
```
Returns the width of the canvas, in pixels.

##### method: getHeight
```c
uint32_t getHeight(DOME_Context ctx)
```
Returns the height of the canvas, in pixels.

##### method: line
```c
void line(DOME_Context ctx, int64_t x0, int64_t y0, int64_t x1, int64_t y1, DOME_Color color);
```
Draws a one pixel wide line between `(x0, y0)` and `(x1, y1)`, in the chosen `color`.

##### method: pget
```c
DOME_Color pget(DOME_Context ctx, uint32_t x, uint32_t y)
```
Returns the color of the pixel located at `(x, y)` in the canvas.

##### method: pset
```c
void pset(DOME_Context ctx, uint32_t x, uint32_t y, DOME_Color color)
```
Sets the color of the pixel located at `(x, y)` in the canvas to `color`.

##### method: unsafePset
```c
void unsafePset(DOME_Context ctx, uint32_t x, uint32_t y, DOME_Color color)
```
Sets the color of the pixel located at `(x, y)` in the canvas to `color`.
This function is provided for performance-sensitive applications.
It does not do range checks. If you attempt to set a pixel outside the canvas,
you risk crashing DOME.

## I/O

This set of APIs allows you to access the host filesystem to read files.

### Acquisition

```c
IO_API_v0* io = (IO_API_v0*)DOME_getAPI(API_IO, IO_API_VERSION);
```
#### Methods
##### method: readFile
```c
void* readFile(DOME_Context ctx, const char* path, size_t* length);
```
Synchronously reads the file located at `path` to memory. The size of the file in bytes is stored in the location 
pointed to by `length`. You are responsible for freeing the returned pointer 
when you are done using it.
