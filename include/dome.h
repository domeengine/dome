/* DOME Plugin Header v0.0.1 */

#include <inttypes.h>
#include <stdbool.h>
#ifndef DOME_PLUGIN_H
#define DOME_PLUGIN_H

// Define external for any platform
#if defined _WIN32 || defined __CYGWIN__
    // Exporting...
  #ifdef __GNUC__
    #define DOME_EXPORT __attribute__ ((dllexport))
  #else
    #define DOME_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
  #endif
  #ifdef __GNUC__
    #define DOME_IMPORT __attribute__ ((dllimport))
  #else
    #define DOME_IMPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
  #endif
  #define DOME_INTERNAL
#else
  #if __GNUC__ >= 4
    #define DOME_EXPORT __attribute__ ((visibility ("default")))
    #define DOME_INTERNAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DOME_EXPORT
    #define DOME_INTERNAL
  #endif
  #define DOME_IMPORT
#endif

#ifdef WIN_EXPORT
  #define PUBLIC_EXPORT DOME_EXPORT
#else
  #define PUBLIC_EXPORT DOME_IMPORT
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
typedef struct WrenHandle WrenHandle;
typedef void (*WrenForeignMethodFn)(WrenVM* vm);
typedef void (*WrenFinalizerFn)(void* data);

typedef enum {
  WREN_TYPE_BOOL,
  WREN_TYPE_NUM,
  WREN_TYPE_FOREIGN,
  WREN_TYPE_LIST,
  WREN_TYPE_MAP,
  WREN_TYPE_NULL,
  WREN_TYPE_STRING,

  // The object is of a type that isn't accessible by the C API.
  WREN_TYPE_UNKNOWN
} WrenType;

typedef enum
{
  WREN_RESULT_SUCCESS,
  WREN_RESULT_COMPILE_ERROR,
  WREN_RESULT_RUNTIME_ERROR
} WrenInterpretResult;

#endif

typedef DOME_Result (*DOME_Plugin_Hook) (DOME_Context context);
typedef WrenForeignMethodFn DOME_ForeignFn;
typedef WrenFinalizerFn DOME_FinalizerFn;




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
  void (*ensureSlots)(WrenVM* vm, int slotCount);

  void (*setSlotNull)(WrenVM* vm, int slot);
  void (*setSlotBool)(WrenVM* vm, int slot, bool value);
  void (*setSlotDouble)(WrenVM* vm, int slot, double value);
  void (*setSlotString)(WrenVM* vm, int slot, const char* text);
  void (*setSlotBytes)(WrenVM* vm, int slot, const char* data, size_t length);
  void* (*setSlotNewForeign)(WrenVM* vm, int slot, int classSlot, size_t length);
  void (*setSlotNewList)(WrenVM* vm, int slot);
  void (*setSlotNewMap)(WrenVM* vm, int slot);

  DOME_Context (*getUserData)(WrenVM* vm);
  bool (*getSlotBool)(WrenVM* vm, int slot);
  double (*getSlotDouble)(WrenVM* vm, int slot);
  const char* (*getSlotString)(WrenVM* vm, int slot);
  const char* (*getSlotBytes)(WrenVM* vm, int slot, int* length);
  void* (*getSlotForeign)(WrenVM* vm, int slot);

  void (*abortFiber)(WrenVM* vm, int slot);
  int (*getSlotCount)(WrenVM* vm);
  WrenType (*getSlotType)(WrenVM* vm, int slot);

  int (*getListCount)(WrenVM* vm, int slot);
  void (*getListElement)(WrenVM* vm, int listSlot, int index, int elementSlot);
  void (*setListElement)(WrenVM* vm, int listSlot, int index, int elementSlot);
  void (*insertInList)(WrenVM* vm, int listSlot, int index, int elementSlot);

  int (*getMapCount)(WrenVM* vm, int slot);
  bool (*getMapContainsKey)(WrenVM* vm, int mapSlot, int keySlot);
  void (*getMapValue)(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
  void (*setMapValue)(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
  void (*removeMapValue)(WrenVM* vm, int mapSlot, int keySlot, int removedValueSlot);

  void (*getVariable)(WrenVM* vm, const char* module, const char* name, int slot);
  WrenHandle* (*getSlotHandle)(WrenVM* vm, int slot);
  void (*setSlotHandle)(WrenVM* vm, int slot, WrenHandle* handle);
  void (*releaseHandle)(WrenVM* vm, WrenHandle* handle);

  bool (*hasVariable)(WrenVM* vm, const char* module, const char* name);
  bool (*hasModule)(WrenVM* vm, const char* module);

  WrenInterpretResult (*call)(WrenVM* vm, WrenHandle* method);
  WrenInterpretResult (*interpret)(WrenVM* vm, const char* module, const char* source);
} WREN_API_v0;

typedef struct {
  DOME_Result (*registerModule)(DOME_Context ctx, const char* name, const char* source);
  DOME_Result (*registerFn)(DOME_Context ctx, const char* name, const char* signature, DOME_ForeignFn method);
  DOME_Result (*registerClass)(DOME_Context ctx, const char* moduleName, const char* className, DOME_ForeignFn allocate, DOME_FinalizerFn finalize);
  void (*lockModule)(DOME_Context ctx, const char* name);
  DOME_Context (*getContext)(WrenVM* vm);
  void (*log)(DOME_Context ctx, const char* text, ...);
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

typedef void (*CHANNEL_mix)(CHANNEL_REF ref, float* buffer, size_t requestedSamples);
typedef void (*CHANNEL_callback)(CHANNEL_REF ref, WrenVM* vm);

typedef struct {
  CHANNEL_REF (*channelCreate)(DOME_Context ctx, CHANNEL_mix mix, CHANNEL_callback update, CHANNEL_callback finish, void* userdata);
  CHANNEL_STATE (*getState)(CHANNEL_REF ref);
  void (*setState)(CHANNEL_REF ref, CHANNEL_STATE state);
  void (*stop)(CHANNEL_REF ref);
  void* (*getData)(CHANNEL_REF ref);
} AUDIO_API_v0;

typedef void* (*DOME_getAPIFunction)(API_TYPE api, int version);
PUBLIC_EXPORT void* DOME_getAPI(API_TYPE api, int version);


// Helper macros to abstract the api->method

#define DOME_registerModule(ctx, name, src) api->registerModule(ctx, name, src)
#define DOME_registerClass(ctx, module, className, allocate, finalize) api->registerClass(ctx, module, className, allocate, finalize)
#define DOME_registerFn(ctx, module, signature, method) \
  api->registerFn(ctx, module, signature, PLUGIN_method_wrap_##method)
#define DOME_lockModule(ctx, module) api->lockModule(ctx, module)
#define DOME_getContext(vm) (api->getContext(vm))

#define DOME_log(ctx, ...) (api->log(ctx, ##__VA_ARGS__))

#define PLUGIN_method(name, ctx, vm) \
  static void PLUGIN_method_##name(DOME_Context ctx, WrenVM* vm); \
  DOME_EXPORT void PLUGIN_method_wrap_##name(WrenVM* vm) { \
    DOME_Context ctx = DOME_getContext(vm); \
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
