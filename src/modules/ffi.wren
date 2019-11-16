class WrenType {
  static BOOL { 0 }
  static NUM { 1 }
  static FOREIGN { 2 }
  static LIST { 3 }
  static NULL { 4 }
  static STRING { 5 }
  static UNKNOWN { -1 }
}

foreign class ModuleHandle {
  construct init(libName) {}
}

class Module {
  construct init(libName){
    _functions = {}
    _handle = ModuleHandle.init(libName)
  }

  static load(moduleName, libName) {
    if (!__modules) {
      __modules = {}
    }
    __modules[moduleName] = Module.init(libName)
    return __modules[moduleName]
  }

  static unload(moduleName) {
    if (__modules) {
      __modules[moduleName] = null
    }
  }

  call(fnName, argPairs) {
    if (_functions[fnName]) {
      return _functions[fnName].call(argPairs)
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

foreign class Function {
  construct bind(module, fnName, returnType, args) {}
  call(args) {
    // TODO: Type assert
    return f_call(args)
  }

  foreign f_call(argsList)
}

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
  foreign asString()
  // foreign asBytes(size)
}
