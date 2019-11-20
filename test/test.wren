import "ffi" for Module, Struct
System.print("FFI Test Module")

var module = Module.load("add", "libadd.so")
module.bind("add", "int", ["int", "int"])
module.bind("getSource", "pointer", [])
var ptr = module.call("getSource", [])
System.print(ptr.asBytes(10))

System.print(module.call("add", [1, 2]))
module.bind("printOut", "void", ["pointer"])
module.call("printOut", ["Hello world\n"])
module.call("printOut", [ptr])

module.bind("printFloat", "void", ["float"])
module.call("printFloat", [4096])

Struct.declare("MiniBlob", ["int"])
Struct.declare("Blob", ["int", "MiniBlob"])
module.bind("printData", "void", ["Blob"])

var miniStruct = Struct.init("MiniBlob", [1024])
var struct = Struct.init("Blob", [42, miniStruct])

var sameStruct = struct.getValue(1).getValue(0)
System.print(sameStruct)
System.print("Struct value: %(struct.getValue(0))")
module.call("printData", [struct])

