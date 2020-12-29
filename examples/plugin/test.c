#include <stdio.h>
#include <stdbool.h>
#include "dome.h"

DOME_API_v0* api;
WREN_API_v0* wren;

size_t i;
bool flag = false;
const char* source = "class Test {\n"
                  "static begin() { System.print(\"Begun!\") }\n"
                  "foreign static end(value)\n"
                  "foreign static empty\n"
                  "foreign static boolean\n"
                  "foreign static number\n"
                  "foreign static string\n"
                  "foreign static bytes\n"
                  "}";

// DOME_PLUGIN_construct();

DOME_PLUGIN_method(end, context) {
  flag = GET_BOOL(1);
  RETURN_BOOL(flag);
};

DOME_PLUGIN_method(value, context) {
  RETURN_NUMBER(i);
}
DOME_PLUGIN_method(text, context) {
  RETURN_STRING("WORDS");
}
DOME_PLUGIN_method(empty, context) {
  RETURN_NULL();
}
DOME_PLUGIN_method(boolean, context) {
  RETURN_BOOL(flag);
}
DOME_PLUGIN_method(bytes, context) {
  char bytes[6] = { 65, 66, 67, 68, 69, 70 };
  RETURN_BYTES(bytes, (i / 60) < 5 ? (i / 60) : 5);
}


DOME_PLUGIN_init(context) {
  api = apiFn(API_DOME, DOME_API_VERSION);
  wren = apiFn(API_WREN, WREN_API_VERSION);

  printf("init hook triggered\n");
  i = 0;
  DOME_registerModule(context, "external", source);
  DOME_registerFn(context, "external", "static Test.end(_)", end);
  DOME_registerFn(context, "external", "static Test.number", value);
  DOME_registerFn(context, "external", "static Test.string", text);
  DOME_registerFn(context, "external", "static Test.boolean", boolean);
  DOME_registerFn(context, "external", "static Test.empty", empty);
  DOME_registerFn(context, "external", "static Test.bytes", bytes);
  return DOME_RESULT_SUCCESS;
}
DOME_PLUGIN_shutdown(context) {
  printf("shutdown hook triggered\n");
  return DOME_RESULT_SUCCESS;
}

DOME_PLUGIN_preupdate(context) {
  i++;
  return DOME_RESULT_SUCCESS;
}


/*
DOME_PLUGIN_postupdate(context) {
  printf("postpdate hook triggered\n");
}
DOME_PLUGIN_predraw(context) {
  printf("predraw hook triggered\n");
}
DOME_PLUGIN_postdraw(context) {
  printf("postdraw hook triggered\n");
}
*/
