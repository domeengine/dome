/* DOME Plugin Header v0.0.1 */

#ifndef DOME_PLUGIN_H
#define DOME_PLUGIN_H

typedef struct DOME_Context_t DOME_Context;
typedef enum {
  DOME_RESULT_SUCCESS,
  DOME_RESULT_FAILURE,
  DOME_RESULT_UNKNOWN
} DOME_Result;

typedef DOME_Result (*DOME_VM_Handler) (DOME_Context* context);


// DOME_Result DOME_Plugin_Init(DOME_Context* context);

#endif
