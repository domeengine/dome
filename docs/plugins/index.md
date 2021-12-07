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
   - [Core](core)
   - [Wren](wren)
   - [Audio](audio)
   - [Bitmap](bitmap)
   - [Canvas](canvas)
   - [I/O](io)
 


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

Below is a table explaining the available services and their purpose.

| Service                 | Description            |
|-------------------------|------------------------|
| [Core](core)            | Engine utilities and module registration                   |
| [Wren](wren)            | A subset of the Wren API for working with foreign classes. |
| [Audio](audio)          | Access DOME's audio engine to provide your own audio.      |
| [Bitmap](bitmap)        | Load images and handle bitmap data.                        |
| [Canvas](canvas)        | Draw to DOME's built-in canvas.                            |
| [I/O](io)               | Access the host filesystem.                                |
