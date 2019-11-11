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
