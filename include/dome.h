/* DOME Plugin Header v0.0.1 */

#include <inttypes.h>
#include <stdbool.h>
#ifndef DOME_PLUGIN_H
#define DOME_PLUGIN_H

// Define external for any platform
#if defined _WIN32 || defined __CYGWIN__
  #ifdef WIN_EXPORT
    // Exporting...
    #ifdef __GNUC__
      #define DOME_EXPORTED __attribute__ ((dllexport))
    #else
      #define DOME_EXPORTED __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DOME_EXPORTED __attribute__ ((dllimport))
    #else
      #define DOME_EXPORTED __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DOME_NOT_EXPORTED
#else
  #if __GNUC__ >= 4
    #define DOME_EXPORTED __attribute__ ((visibility ("default")))
    #define DOME_NOT_EXPORTED  __attribute__ ((visibility ("hidden")))
  #else
    #define DOME_EXPORTED
    #define DOME_NOT_EXPORTED
  #endif
#endif


typedef enum {
  API_DOME,
  API_WREN,
  API_AUDIO
} API_TYPE;

#define DOME_API_VERSION 0
#define WREN_API_VERSION 0
#define AUDIO_API_VERSION 0

// Opaque context pointer
typedef void* DOME_Context;

typedef enum {
  DOME_RESULT_SUCCESS,
  DOME_RESULT_FAILURE,
  DOME_RESULT_UNKNOWN
} DOME_Result;

#ifndef wren_h
// If the wren header is not in use, we forward declare some types we need.
typedef struct WrenVM WrenVM;
typedef void (*WrenForeignMethodFn)(WrenVM* vm);
typedef void (*WrenFinalizerFn)(void* data);
typedef struct {
  WrenForeignMethodFn allocate;
  WrenFinalizerFn finalize;
} WrenForeignClassMethods;
#endif

typedef DOME_Result (*DOME_Plugin_Hook) (DOME_Context context);
typedef void (*DOME_ForeignFn)(WrenVM* vm);
typedef WrenForeignClassMethods (*DOME_BindClassFn) (const char* className);




// DO NOT CHANGE ORDER OF THESE STRUCTS, to preserve ABI.
typedef struct {
  const char* name;
  DOME_Plugin_Hook preUpdate;
  DOME_Plugin_Hook postUpdate;
  DOME_Plugin_Hook preDraw;
  DOME_Plugin_Hook postDraw;
  DOME_Plugin_Hook onShutdown;
} PLUGIN;

typedef struct {
  void (*setSlotNull)(WrenVM* vm, int slot);
  void (*setSlotBool)(WrenVM* vm, int slot, bool value);
  void (*setSlotDouble)(WrenVM* vm, int slot, double value);
  void (*setSlotString)(WrenVM* vm, int slot, const char* text);
  void (*setSlotBytes)(WrenVM* vm, int slot, const char* data, size_t length);
  void* (*setSlotNewForeign)(WrenVM* vm, int slot, int classSlot, size_t length);

  DOME_Context (*getUserData)(WrenVM* vm);
  bool (*getSlotBool)(WrenVM* vm, int slot);
  double (*getSlotDouble)(WrenVM* vm, int slot);
  const char* (*getSlotString)(WrenVM* vm, int slot);
  const char* (*getSlotBytes)(WrenVM* vm, int slot, int* length);
  void* (*getSlotForeign)(WrenVM* vm, int slot);

  void (*abortFiber)(WrenVM* vm, int slot);
} WREN_API_v0;

typedef struct {
  DOME_Result (*registerModule)(DOME_Context ctx, const char* name, const char* source);
  DOME_Result (*registerFn)(DOME_Context ctx, const char* name, const char* signature, DOME_ForeignFn method);
  DOME_Result (*registerBindFn)(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn);
} DOME_API_v0;

typedef uint64_t CHANNEL_ID;
typedef struct {
  CHANNEL_ID id;
  void* engine;
} CHANNEL_REF;

typedef struct CHANNEL_t CHANNEL;
typedef enum {
  CHANNEL_INVALID,
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_DEVIRTUALIZE,
  CHANNEL_LOADING,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED,
  CHANNEL_VIRTUALIZING,
  CHANNEL_VIRTUAL,
  CHANNEL_LAST
} CHANNEL_STATE;

typedef void (*CHANNEL_mix)(CHANNEL* channel, float* buffer, size_t requestedSamples);
typedef void (*CHANNEL_callback)(WrenVM* vm, CHANNEL* channel);

typedef struct {
  CHANNEL_REF (*channelCreate)(DOME_Context ctx, CHANNEL_mix mix, CHANNEL_callback update, CHANNEL_callback finish, void* userdata);
  void (*setState)(DOME_Context ctx, CHANNEL_REF ref, CHANNEL_STATE state);
  void (*stop)(DOME_Context ctx, CHANNEL_REF ref);
} AUDIO_API_v0;

typedef void* (*DOME_getAPIFunction)(API_TYPE api, int version);
DOME_EXPORTED void* DOME_getAPI(API_TYPE api, int version);


// Helper macros to abstract the api->method

#define DOME_registerModule(ctx, name, src) api->registerModule(ctx, name, src)
#define DOME_registerBindFn(ctx, module, fn) api->registerBindFn(ctx, module, fn)
#define DOME_registerFn(ctx, module, signature, method) \
  api->registerFn(ctx, module, signature, PLUGIN_method_wrap_##method)

#define PLUGIN_method(name, ctx, vm) \
  static void PLUGIN_method_##name(DOME_Context ctx, WrenVM* vm); \
  DOME_EXPORTED void PLUGIN_method_wrap_##name(WrenVM* vm) { \
    DOME_Context ctx = (DOME_Context) wren->getUserData(vm); \
    PLUGIN_method_##name(ctx, vm);\
  } \
  static void PLUGIN_method_##name(DOME_Context ctx, WrenVM* vm)

#define GET_BOOL(slot) wren->getSlotBool(vm, slot)
#define GET_NUMBER(slot) wren->getSlotDouble(vm, slot)
#define GET_STRING(slot) wren->getSlotString(vm, slot)
#define GET_BYTES(slot, length) wren->getSlotBytes(vm, slot, length)
#define GET_FOREIGN(slot) wren->getSlotForeign(vm, slot)
#define RETURN_NULL() wren->setSlotNull(vm, 0);
#define RETURN_NUMBER(value) wren->setSlotDouble(vm, 0, value);
#define RETURN_BOOL(value) wren->setSlotBool(vm, 0, value);
#define RETURN_STRING(value) wren->setSlotString(vm, 0, value);
#define RETURN_BYTES(value, length) wren->setSlotBytes(vm, 0, value, length);

#define THROW_ERROR(message) \
  do { \
    wren->setSlotString(vm, 0, message); \
    wren->abortFiber(vm, 0); \
  } while (false)

#endif
