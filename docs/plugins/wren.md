[< Back](.)

Wren
===============

You have access to a subset of the [Wren slot API](https://wren.io/embedding/slots-and-handles.html) in order to access parameters and return values in foreign methods.
The methods are incredibly well documented in the [Wren public header](https://github.com/wren-lang/wren/blob/main/src/include/wren.h), so we will not be documenting the functions here.
 
You do not need to include the `wren.h` header in your application, as `dome.h` includes everything you need.

  * [Acquisition](#acquistion)
  * [Module Embedding](#module-embedding)

## Acquisition

```c
WREN_API_v0* wren = (WREN_API_v0*)DOME_getAPI(API_WREN, WREN_API_VERSION);
```

## Methods
This is a list of provided methods:
```c
           void     ensureSlots(WrenVM* vm, int slotCount);
           void     setSlotNull(WrenVM* vm, int slot);
           void     setSlotBool(WrenVM* vm, int slot, bool value);
           void     setSlotDouble(WrenVM* vm, int slot, double value);
           void     setSlotString(WrenVM* vm, int slot, const char* text);
           void     setSlotBytesWrenVM* vm, int slot, const char* data, size_t length);
           void*    setSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t length);
           bool     getSlotBool(WrenVM* vm, int slot);
           double   getSlotDouble(WrenVM* vm, int slot);    
     const char*    getSlotString(WrenVM* vm, int slot);   
     const char*    getSlotBytes(WrenVM* vm, int slot, int* length);                   
           void*    getSlotForeign(WrenVM* vm, int slot);                   
 
        WrenType    getSlotType(WrenVM* vm, int slot);
 
           void     setSlotNewList(WrenVM* vm, int slot);
           int      getListCount(WrenVM* vm, int slot);
           void     getListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
           void     setListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
           void     insertInList(WrenVM* vm, int listSlot, int index, int elementSlot);
 
           void     setSlotNewMap(WrenVM* vm, int slot);
           int      getMapCount(WrenVM* vm, int slot);
           bool     getMapContainsKey(WrenVM* vm, int mapSlot, int keySlot);
           void     getMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
           void     setMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);
           void     removeMapValue(WrenVM* vm, int mapSlot, int keySlot, int removedValueSlot);
 

WrenInterpretResult interpret(WrenVM* vm, const char* module, const char* source);
WrenInterpretResult call(WrenVM* vm, WrenHandle* method);

           bool     hasModule(WrenVM* vm, const char* module);
           bool     hasVariable(WrenVM* vm, const char* module, const char* name);
           void     getVariable(WrenVM* vm, const char* module, const char* name, int slot);
     WrenHandle*    getSlotHandle(WrenVM* vm, int slot);
           void     setSlotHandle(WrenVM* vm, int slot, WrenHandle* handle);
           void     releaseHandle(WrenVM* vm, WrenHandle* handle);
           void     abortFiber(WrenVM* vm, int slot);
```

## Module Embedding

If your plugin registers a Wren module, you can embed the source of that module in your plugin by using DOME's built-in `embed` subcommand, which will convert it into a C include file.

```sh
$ dome embed sourceFile [moduleVariableName] [destinationFile]
```

Example:

```sh
$ dome embed external.wren sourceModule external.wren.inc
```

This command will use `external.wren` to generate `external.wren.inc`, which contains the variable `sourceModule` for including in C/C++ source code.
