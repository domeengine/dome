import "ffi" for Library, Struct
System.print("FFI Test Library")

var library = Library.load("add", "libadd.so")
library.bind("add", "int", ["int", "int"])
library.bind("getSource", "pointer", [])
var ptr = library.call("getSource", [])
System.print(ptr.asBytes(10))

System.print(library.call("add", [1, 2]))
library.bind("printOut", "void", ["pointer"])
library.call("printOut", ["Hello world\n"])
library.call("printOut", [ptr])

library.bind("printFloat", "void", ["float"])
library.call("printFloat", [4096])

Struct.declare("MiniBlob", ["int"])
Struct.declare("Blob", ["int", "MiniBlob"])
library.bind("printData", "void", ["Blob"])

var miniStruct = Struct.init("MiniBlob", [1024])
var struct = Struct.init("Blob", [42, miniStruct])

var sameStruct = struct.getValue(1).getValue(0)
System.print(sameStruct)
System.print("Struct value: %(struct.getValue(0))")
library.call("printData", [struct])

