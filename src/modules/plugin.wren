class Plugin {
  static load(path) {
    System.print(Platform.name)
    if (Platform.name.startsWith("Windows")) {
      path = path + ".dll"
    } else if (Platform.name.startsWith("Linux")) {
      path = path + ".so"
    } else if (Platform.name.startsWith("Mac OS X")) {
      path = path + ".dylib"
    }
    return f_load(path)
  }
  foreign static f_load(path)
//  foreign static unload(path)
}

