import "dome" for Platform

class Plugin {
  static load(path) {
    if (Platform.name.startsWith("Windows")) {
      path = path + ".dll"
    } else if (Platform.name.startsWith("Mac OS X")) {
      path = path + ".dylib"
    } else {
      // Linux and other, as .so is a more generic format
      path = path + ".so"
    }
    return f_load(path)
  }

  foreign static f_load(path)
}

