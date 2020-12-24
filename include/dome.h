/* DOME Plugin Header v0.0.1 */

#ifndef DOME_PLUGIN_H
#define DOME_PLUGIN_H


// Define DOME_EXPORTED for any platform
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
  DOME_registerFnImpl(ctx, module, signature, DOME_PLUGIN_method_wrap_##_name)


#define DOME_PLUGIN_method(name, context, vm) \
  static void DOME_PLUGIN_method_##_name(DOME_Context ctx, void* vm); \
  DOME_EXPORTED void DOME_PLUGIN_method_wrap_##_name(void* vm) { \
    DOME_Context ctx = (DOME_Context) DOME_getContext(vm); \
    DOME_PLUGIN_method_##_name(ctx, vm);\
  } \
  static void DOME_PLUGIN_method_##_name(DOME_Context ctx, void* vm)

#define DOME_PLUGIN_init(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnInit(DOME_Context ctx)

#define DOME_PLUGIN_shutdown(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnShutdown(DOME_Context ctx)

#define DOME_PLUGIN_preupdate(ctx) \
  DOME_EXPORTED DOME_Result DOME_hookOnPreUpdate(DOME_Context ctx)


typedef void (*DOME_ForeignFn)(void* vm);
DOME_EXPORTED DOME_Result DOME_registerModule(DOME_Context ctx, const char* name, const char* source);

DOME_EXPORTED DOME_Result DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method);

DOME_EXPORTED DOME_Context DOME_getContext(void* vm);

#endif
