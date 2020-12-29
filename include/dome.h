/* DOME Plugin Header v0.0.1 */

#ifndef DOME_PLUGIN_H
#define DOME_PLUGIN_H

// Make sure we have access to a bool type for C89
// which handles mode of the bool semantics.
#if __bool_true_false_are_defined == 0
typedef enum {
  false,
  true
} bool;
#endif

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


// Opaque context pointer
typedef void* DOME_Context;

typedef enum {
  DOME_RESULT_SUCCESS,
  DOME_RESULT_FAILURE,
  DOME_RESULT_UNKNOWN
} DOME_Result;

#define DOME_registerFn(ctx, module, signature, method) \
  DOME_registerFnImpl(ctx, module, signature, DOME_PLUGIN_method_wrap_##method)


#define DOME_PLUGIN_method(name, context) \
  static void DOME_PLUGIN_method_##name(DOME_Context ctx, void* vm); \
  DOME_EXPORTED void DOME_PLUGIN_method_wrap_##name(void* vm) { \
    DOME_Context ctx = (DOME_Context) DOME_getContext(vm); \
    DOME_PLUGIN_method_##name(ctx, vm);\
  } \
  static void DOME_PLUGIN_method_##name(DOME_Context ctx, void* vm)

#define DOME_PLUGIN_init(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnInit(DOME_Context ctx)

#define DOME_PLUGIN_shutdown(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnShutdown(DOME_Context ctx)

#define DOME_PLUGIN_preupdate(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnPreUpdate(DOME_Context ctx)


typedef void (*DOME_ForeignFn)(void* vm);
typedef WrenForeignClassMethods (*DOME_BindClassFn) (const char* className);

DOME_EXPORTED DOME_Result DOME_registerModule(DOME_Context ctx, const char* name, const char* source);

DOME_EXPORTED DOME_Result DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method);

DOME_EXPORTED DOME_Context DOME_getContext(void* vm);

typedef enum {
  DOME_SLOT_TYPE_NULL,
  DOME_SLOT_TYPE_BOOL,
  DOME_SLOT_TYPE_NUMBER,
  DOME_SLOT_TYPE_STRING,
  DOME_SLOT_TYPE_BYTES,
  DOME_SLOT_TYPE_LIST,
  DOME_SLOT_TYPE_MAP,
  DOME_SLOT_TYPE_HANDLE,
  DOME_SLOT_TYPE_FOREIGN
} DOME_SLOT_TYPE;

typedef struct {
  union {
    double number;
    char* text;
    bool boolean;
    struct {
      char* data;
      size_t len;
    } bytes;
  } as;
} DOME_SLOT_VALUE;


DOME_EXPORTED bool DOME_setSlot(void* vm, size_t slot, DOME_SLOT_TYPE type, ...);
DOME_EXPORTED DOME_SLOT_VALUE DOME_getSlot(void* vm, size_t slot, DOME_SLOT_TYPE type);

#define RETURN_NULL() DOME_setSlot(vm, 0, DOME_SLOT_TYPE_NULL)
#define RETURN_BOOL(value) DOME_setSlot(vm, 0, DOME_SLOT_TYPE_BOOL, (bool)value)
#define RETURN_NUMBER(value) DOME_setSlot(vm, 0, DOME_SLOT_TYPE_NUMBER, (double)value)
#define RETURN_STRING(value) DOME_setSlot(vm, 0, DOME_SLOT_TYPE_STRING, (char*)value)
#define RETURN_BYTES(value, length) DOME_setSlot(vm, 0, DOME_SLOT_TYPE_BYTES, (char*)value, (size_t)length)

#define GET_BOOL(slot) DOME_getSlot(vm, slot, DOME_SLOT_TYPE_BOOL).as.boolean
#define GET_NUMBER(slot) DOME_getSlot(vm, slot, DOME_SLOT_TYPE_NUMBER).as.number
#define GET_STRING(slot) DOME_getSlot(vm, slot, DOME_SLOT_TYPE_STRING).as.text
#define GET_BYTES(slot) DOME_getSlot(vm, slot, DOME_SLOT_TYPE_BYTES).as.bytes

#endif
