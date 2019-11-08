class Process {
  foreign static f_exit(n)
  static exit(n) { f_exit(n) }
  static exit() {
    exit(0)
  }
}

class WrenType {
  static BOOL { 0 }
  static NUM { 1 }
  static FOREIGN { 2 }
  static LIST { 3 }
  static NULL { 4 }
  static STRING { 5 }
  static UNKNOWN { -1 }
}


foreign class ForeignModule {
  construct init(moduleName, libName){}
  static load(moduleName, libName) {
    if (!__modules) {
      __modules = {}
    }
    __modules[moduleName] = ForeignModule.init(moduleName, libName)
    return __modules[moduleName]
  }
  static unload(moduleName) {
    if (__modules) {
      __modules[moduleName] = null
    }
  }

  call(fn, returnType, argPairs) {
    // TODO type assertions
    f_call(fn, returnType, argPairs)
  }

  foreign f_call(fnName, returnType, argPairs)
}
