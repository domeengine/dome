// FFI Library

// Private
foreign class LibraryHandle {
  construct init(libName) {}
}

class Library {
  construct init(libName){
    _functions = {}
    _handle = LibraryHandle.init(libName)
  }

  static get(libraryName) {
    if (!__libraries) {
      __libraries = {}
    }
    return __libraries[libraryName]
  }

  static load(libraryName, libName) {
    if (!__libraries) {
      __libraries = {}
    }
    if (!__libraries[libraryName]) {
      __libraries[libraryName] = Library.init(libName)
    }
    return __libraries[libraryName]
  }

  static unload(libraryName) {
    if (__libraries) {
      __libraries[libraryName] = null
    }
  }

  call(fnName, params) {
    if (_functions[fnName]) {
      return _functions[fnName].call(params)
    }
  }

  bind(fnName, retType, paramTypeList) {
    if (!_functions) {
      _functions = {}
    }

    // handle struct binding
    paramTypeList = paramTypeList.map {|typeName|
      if (typeName is String && Struct.getType(typeName)) {
        return Struct.getType(typeName)
      }
      return typeName
    }.toList

    if (retType is String && Struct.getType(retType)) {
      retType = Struct.getType(retType)
    }

    _functions[fnName] = Function.bind(_handle, fnName, retType, paramTypeList)
  }
}

// Private
foreign class Function {
  construct bind(library, fnName, returnType, args) {}
  call(args) {
    // TODO: Type assert
    return f_call(args)
  }

  foreign f_call(argsList)
}

// Private
foreign class StructTypeData {
  construct bind(list, empty) {}
}

foreign class Struct {
  construct bind(type, values) {}
  static init(type, values) {
    if (type is String) {
      type = __types[type]
    }
    if (!type || !(type is StructTypeData)) {
      Fiber.abort("Invalid Struct Type")
    }
    return Struct.bind(type, values)
  }

  static declare(name, types) {
    if (!__types) {
      __types = {}
    }
    types = types.map {|typeName|
      if (typeName is String && __types[typeName]) {
        return __types[typeName]
      }
      return typeName
    }.toList
    __types[name] = StructTypeData.bind(types, null)
    return __types[name]
  }

  static getType(name) {
    if (!__types) {
      __types = {}
    }
    return __types[name]
  }

  foreign getValue(memberIndex)
}

foreign class Pointer {
  construct new() {}

  foreign asString()
  foreign static reserve(bytes)
  foreign free()
  foreign asBytes(size)
}
