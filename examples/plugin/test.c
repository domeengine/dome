#include <stdio.h>
#include <stdbool.h>
#include "dome.h"

size_t i;
const char* source = "class Test {\n"
                  "static begin() { System.print(\"Begun!\") }\n"
                  "foreign static end()\n"
                  "foreign static value\n"
                  "}";

DOME_PLUGIN_method(end, context) {
  printf("foreign method\n");
}

DOME_PLUGIN_method(value, context) {
  RETURN_NUMBER(i);
}

DOME_PLUGIN_init(context) {
  printf("init hook triggered\n");
  i = 0;
  DOME_registerModule(context, "external", source);
  DOME_registerFn(context, "external", "static Test.end()", end);
  DOME_registerFn(context, "external", "static Test.value", value);
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
