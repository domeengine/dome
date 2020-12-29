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
  API_WREN
} API_TYPE;

#define DOME_API_VERSION 0
#define WREN_API_VERSION 0

// Opaque context pointer
typedef void* DOME_Context;

#ifndef wren_h
// If the wren header is not in use, we forward declare some types we need.
typedef struct WrenVM WrenVM;
typedef struct WrenForeignClassMethods WrenForeignClassMethods;
#endif

typedef WrenForeignClassMethods (*DOME_BindClassFn) (const char* className);
typedef void (*DOME_ForeignFn)(void* vm);

typedef enum {
  DOME_RESULT_SUCCESS,
  DOME_RESULT_FAILURE,
  DOME_RESULT_UNKNOWN
} DOME_Result;


// DO NOT CHANGE ORDER OF THESE, to preserve ABI.
typedef struct {
  void (*setSlotNull)(WrenVM* vm, int slot);
  void (*setSlotBool)(WrenVM* vm, int slot, bool value);
  void (*setSlotDouble)(WrenVM* vm, int slot, double value);
  void (*setSlotString)(WrenVM* vm, int slot, const char* text);
  void (*setSlotBytes)(WrenVM* vm, int slot, const char* data, size_t length);

  DOME_Context (*getUserData)(WrenVM* vm);
  bool (*getSlotBool)(WrenVM* vm, int slot);
  double (*getSlotDouble)(WrenVM* vm, int slot);
  const char* (*getSlotString)(WrenVM* vm, int slot);
  const char* (*getSlotBytes)(WrenVM* vm, int slot, int* length);

  void (*abortFiber)(WrenVM* vm, int slot);
} WREN_API_v0;

typedef struct {
  DOME_Result (*registerModule)(DOME_Context ctx, const char* name, const char* source);
  DOME_Result (*registerFnImpl)(DOME_Context ctx, const char* name, const char* signature, DOME_ForeignFn method);
  DOME_Result (*registerBindFn)(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn);
} DOME_API_v0;

typedef void* (*DOME_getAPI)(API_TYPE api, int version);
DOME_EXPORTED void* DOME_getApiImpl(API_TYPE api, int version);

#define DOME_registerModule(ctx, name, src) api->registerModule(ctx, name, src)
#define DOME_registerBindFn(ctx, module, fn) api->registerBindFn(ctx, module, fn)
#define DOME_registerFn(ctx, module, signature, method) \
  api->registerFnImpl(ctx, module, signature, DOME_PLUGIN_method_wrap_##method)
#define DOME_getContext(vm) wren->getUserData(vm)

#define DOME_PLUGIN_method(name, context) \
  static void DOME_PLUGIN_method_##name(DOME_Context ctx, void* vm); \
  DOME_EXPORTED void DOME_PLUGIN_method_wrap_##name(void* vm) { \
    DOME_Context ctx = (DOME_Context) wren->getUserData(vm); \
    DOME_PLUGIN_method_##name(ctx, vm);\
  } \
  static void DOME_PLUGIN_method_##name(DOME_Context ctx, void* vm)

#define DOME_PLUGIN_init(apiFn, ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnInit(DOME_getAPI apiFn, DOME_Context ctx)

#define DOME_PLUGIN_shutdown(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnShutdown(DOME_Context ctx)

#define DOME_PLUGIN_preupdate(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnPreUpdate(DOME_Context ctx)

#define GET_BOOL(slot) wren->getSlotBool(vm, slot)
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
