#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "dome.h"

static DOME_API_v0* api;
static WREN_API_v0* wren;

static size_t i;
static bool flag = false;
static const char* source = "foreign class Test {\n"
                  "construct new() { \n "
                  "  System.print(\"New Class!\") \n"
                  "}\n"
                  "static begin() { System.print(\"Begun!\") }\n"
                  "foreign static end(value)\n"
                  "foreign static empty\n"
                  "foreign static boolean\n"
                  "foreign static number\n"
                  "foreign static string\n"
                  "foreign static bytes\n"
                  "}";

DOME_PLUGIN_method(end, ctx, vm) {
  flag = GET_BOOL(1);
  RETURN_BOOL(flag);
};

DOME_PLUGIN_method(value, context, vm) {
  RETURN_NUMBER(i);
}
DOME_PLUGIN_method(text, ctx, vm) {
  RETURN_STRING("WORDS");
}
DOME_PLUGIN_method(empty, ctx, vm) {
  RETURN_NULL();
}
DOME_PLUGIN_method(boolean, ctx, vm) {
  RETURN_BOOL(flag);
}
DOME_PLUGIN_method(bytes, ctx, vm) {
  char bytes[6] = { 65, 66, 67, 68, 69, 70 };
  RETURN_BYTES(bytes, (i / 60) < 5 ? (i / 60) : 5);
}

void TEST_allocate(WrenVM* vm) {
  char* data = (char*)wren->setSlotNewForeign(vm, 0, 0, sizeof(char) * 2);
  if (data != NULL) {
    printf("size allocated\n");
    data[0] = 'Z';
    data[1] = '\0';
  }
}

void TEST_finalizer(void* data) {
  printf("size deallocated: %s\n", (char*)data);
}

WrenForeignClassMethods DOME_PLUGIN_bind(const char* className) {
  WrenForeignClassMethods methods;
  methods.allocate = NULL;
  methods.finalize = NULL;
  if (strcmp(className, "Test") == 0) {
    methods.allocate = TEST_allocate;
    methods.finalize = TEST_finalizer;
  }
  return methods;
}

DOME_PLUGIN_init(DOME_getApi, ctx) {

  api = DOME_getApi(API_DOME, DOME_API_VERSION);
  wren = api->wren;

  printf("init hook triggered\n");
  i = 0;
  DOME_registerModule(ctx, "external", source);
  DOME_registerBindFn(ctx, "external", DOME_PLUGIN_bind);
  DOME_registerFn(ctx, "external", "static Test.end(_)", end);
  DOME_registerFn(ctx, "external", "static Test.number", value);
  DOME_registerFn(ctx, "external", "static Test.string", text);
  DOME_registerFn(ctx, "external", "static Test.boolean", boolean);
  DOME_registerFn(ctx, "external", "static Test.empty", empty);
  DOME_registerFn(ctx, "external", "static Test.bytes", bytes);
  return DOME_RESULT_SUCCESS;
}
DOME_PLUGIN_shutdown(ctx) {
  printf("shutdown hook triggered\n");
  return DOME_RESULT_SUCCESS;
}

DOME_PLUGIN_preupdate(ctx) {
  i++;
  return DOME_RESULT_SUCCESS;
}

DOME_PLUGIN_postupdate(ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_PLUGIN_predraw(ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_PLUGIN_postdraw(ctx) {
  return DOME_RESULT_SUCCESS;
}
