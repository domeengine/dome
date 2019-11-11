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

// foreign class StructHandle {}
foreign class StructTypeData {
  construct bind(list, empty) {}
  foreign getMemberOffset(elementIndex)
}

var struct = StructTypeData.bind(["uint"], null)
/*
class StructType {
  construct declare(typeName, elementList) {
    if (!__types) {
      __types = {}
    }
    _elementNames = []
    var elementTypes = []
    var iter_ = null
    if (_elementNames.count % 2 != 0) {
      Fiber.abort("Invalid key/type pairs provided.")
    }
    _elementIndexes = {}

    var i = 0
    while (iter_ = elementList.iterate(iter_)) {
      var key = elementList.iteratorValue(iter_)
      var value = elementList.iteratorValue(iter_)
      _elementIndex[key] = i
      elementTypes.add(value)
      i = i + 1
    }
    _type = StructTypeData.bind(elementTypes)
    __types[typeName] = this
  }

  getMemberOffset(elementName) {
    return _type.getMemberOffset(_elementIndex[elementName])
  }

}


 */
