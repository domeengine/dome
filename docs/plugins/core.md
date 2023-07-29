[< Back](.)

Core
===============

This API allows your plugin to register modules and provides some basic utilities.

  * Enums
    - [enum: DOME_Result](#enum-dome_result)
  * Function Signatures
    - [function: DOME_ForeignFn](#function-dome_foreignfn)
    - [function: DOME_FinalizerFn](#function-dome_finalizerfn)
  * Methods
    - [method: registerModule](#method-registermodule)
    - [method: registerClass](#method-registerclass)
    - [method: registerFn](#method-registerfn)
    - [method: lockModule](#method-lockmodule)
    - [method: getContext](#method-getcontext)
    - [method: getLastError](#method-getlasterror)
    - [method: log](#method-log)


### Acquisition

```c
DOME_API_v0* core = (DOME_API_v0*)DOME_getAPI(API_DOME, DOME_API_VERSION);
```

### Enums: 

#### enum: DOME_Result

Various methods return an enum of type `DOME_Result`, which indicates success or failure. These are the valid values:

 * `DOME_RESULT_SUCCESS`
 * `DOME_RESULT_FAILURE`
 * `DOME_RESULT_UNKNOWN`

### Function signatures

#### function: DOME_ForeignFn
`DOME_ForeignFn` methods have the signature: `void method(WrenVM* vm)` to match the `WrenForeignMethodFn` type.

#### function: DOME_FinalizerFn
`DOME_FinalizerFn` methods have the signature: `void finalize(void* vm)`, to match the `WrenFinalizerFn` type.

### Methods

#### method: registerModule
```c
DOME_Result registerModule(DOME_Context ctx, 
                           const char* name, 
                           const char* moduleSource)
```
This call registers module `name` with the source code `moduleSource`. You cannot register modules with the same name as DOME's internal modules. These are reserved. 
DOME creates a copy of the `name` and `moduleSource`, so you are able to free the pointers if necessary.
Returns `DOME_RESULT_SUCCESS` if the module was successfully registered, and `DOME_RESULT_FAILURE` otherwise.

#### method: registerClass
```c
DOME_Result registerClass(DOME_Context ctx, 
                          const char* moduleName, 
                          const char* className, 
                          DOME_ForeignFn allocate, 
                          DOME_FinalizerFn finalize)
```
Register the `allocate` and `finalize` methods for `className` in `moduleName`, so that instances of the foreign class can be allocated, and optionally finalized.
The `finalize` method is your final chance to deal with the userdata attached to your foreign class. You won't have VM access inside this method.
DOME creates a copy of the `className`, so you are able to free the pointer if necessary.

Returns `DOME_RESULT_SUCCESS` if the class is registered and `DOME_RESULT_FAILURE` otherwise. Failure will occur if `allocate` method is provided. The `finalize` argument can optionally be `NULL`.


#### method: registerFn
```c
DOME_Result registerFn(DOME_Context ctx, 
                       const char* name, 
                       const char* signature, 
                       DOME_ForeignFn method)
```
Register `method` as the function to call for the foreign method specified by `signature` in the module `name`. 
DOME creates a copy of the `signature`, so you are able to free the pointer if necessary.
Returns `DOME_RESULT_SUCCESS` if the function was successfully registered, and `DOME_RESULT_FAILURE` otherwise.

The format for the `signature` string is as follows:
 * `static` if the method is a static class method, followed by a space, otherwise both are omitted.
 * `ClassName` for the class method being declared, followed by a period (`.`)
 * `methodName` which is the name of the field/method being exposed.
   - If this is a field getter, nothing else is needed.
   - If this is a field setter, add `=(_)`
   - If this is a method, then parenthesis and a comma separated list of underscores (`_`) follow, for the number of arguments the method takes. 
   - You can also use the setter and getter syntax for the class' subscript operator `[]`, which can be defined with one or more parameters.
   - Wren methods can have up to 16 arguments, and are overloaded by arity. For example, `Test.do(_)` is considered different to `Test.do(_,_)` and so on.
   
#### method: lockModule
```c
void lockModule(DOME_Context ctx, const char* name)
```
This marks the module `name` as locked, so that further functions cannot modify it. It is recommended to do this after you have registered all the methods for your module, however there is no requirement to.

#### method: getContext
```c
DOME_Context getContext(WrenVM* vm)
```
This allows foreign functions called by the Wren VM to access the current DOME context, to call various APIs.

#### method: getLastError
```c
DOME_Context getLastError(WrenVM* vm)
```
This returns the last error message reported by a failed plugin API call.
The error message will never be longer than 4096 bytes, including a terminating character.

#### method: log
```c
void log(DOME_Context ctx, const char* text, ...)
```

Using this method allows for formatted output of `text` to the various debugging outputs DOME uses (stdout, a debug console on Windows and a DOME-log.txt file). 

You can use C-style specifiers for the `text` string, as used in the `printf` family of functions.
