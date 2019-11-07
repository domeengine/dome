class Process {
  foreign static f_exit(n)
  static exit(n) { f_exit(n) }
  static exit() {
    exit(0)
  }
}



foreign class ForeignModule {
  construct load(moduleName, libName) {
    if (!__modules) {
      __modules = {}
    }
    __modules[moduleName] = this
  }
  static unload(moduleName) {
    if (__modules) {
      __modules[moduleName] = null
    }
  }
}
