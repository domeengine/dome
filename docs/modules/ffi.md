[< Back](.)

ffi
================

The `ffi` module provides an interface to dynamically loaded libraries (DLLs) written and compiled from other languages.
It uses `libffi` so you can define the functions provided by the DLL and then call methods from it.

The purpose is to allow you to hook into other libraries to extend the functionality of DOME, when needed.

Accessing DLLs and using libffi calls is a very low level operation, and comes with certain caveats:
 * Calls using FFI will not be very performant.
 * There is no type-checking between your function definition and the function being called. Mistakes will lead to crashes and unexpected behaviour.
 * You will have to be aware of the memory implications of your calls into the DLL.
 * There are security risks when relying on DLLs to provide functionality.

# Types

Types are referred to as a string of the C type, except for "pointer" and the names of pre-defined structs.

It contains the following classes:

* [Library](#library)

## Library

### Static Methods

#### `static load(shortName: String, libraryName: String): Library`
Given a `libraryName` which is valid and on the path in your execution environment, DOME will load that DLL for future use, 
and return you a Library object representing it. It will be assigned the given `shortName`, for future retrieval if necessary.

#### `static get(shortName: String): Library`
If a library was loaded with the given `shortName`, return it. This is useful for retrieving previously loaded library.

#### `static unload(shortName: String): Void`
Unload the `shortName`ed DLL. After this, all functions within that library are inaccessible and invalidated.

### Instance Methods

#### `bind(fnName: String, retType: String, paramTypeList: String)[]): Void`
This defines a function named `fnName` which returns a value of `retType` and is called with parameters in order of `paramTypeList`.

#### `call(fnName: String, params: Any[]): Any`
Calls the function `fnName` with the parameters in the `params` list.
There is minimal type checking performed here, and you are responsible for passing in the correct types.

## Struct

### Static Methods

#### `static declare(typeName: String, fieldTypeList: String): StructType`
Defines a structType which can be passed to functions in a library.

#### `static init(typeName: String, params: Any[]): Struct`
Initialises a struct with the given parameters, ready to be passed to a function.

### Instance Methods

#### `getValue(index: Number): Any`
Retrieves the value held in the field at the given index.

## Pointer
This represents a pointer to a block of contiguous memory. It can be returned by a function or reference memory reserved by `reserve`.

### Static Methods
#### `reserve(bytes: Number): Pointer`
Reserve `bytes` of memory and return a pointer to it. This pointer can be freed by calling `free`.

### Instance Methods

#### `free(): Void`
Frees memory reserved by `reserve`.

#### `asString(): String`
Treats the pointer as a C-String so it will return all bytes until a null-terminator.

#### `asBytes(size: Number): String`
Treats the pointer as an array of bytes and returns as many bytes as expected.


