class Version {
  foreign static toString

  static major { this.toList[0] }
  static minor { this.toList[1] }
  static patch { this.toList[2] }

  static toList {
    if (!__list) {
      __list = toString.split(".").map {|value| Num.fromString(value) }.toList
    }
    return __list
  }
  static atLeast(version) {
    var values = version.split(".").map {|value| Num.fromString(value) }.toList
    var actual = this.toList
    if (values[0] > actual[0]) {
      return false
    }
    if (values[0] < actual[0]) {
      return true
    }
    if (values.count > 1) {
      if (values[1] > actual[1]) {
        return false
      }
      if (values[1] < actual[1]) {
        return true
      }
    }
    if (values.count > 2) {
      if (values[2] > actual[2]) {
        return false
      }
      if (values[2] < actual[2]) {
        return true
      }
    }
    return true
  }
}




class Process {
  foreign static f_exit(n)
  foreign static f_args

  static arguments {
    var args = Process.f_args
    if (args.count > 1) {
      return args[2..-1]
    }
    return args
  }

  static arguments(needle) {
    var args = Process.arguments
    var result = null
    Fiber.new {
      args.each {|value|
        if (value.contains(needle)) {
          result = value
          // Skip all other values
          Fiber.abort()
        }
      }
    }.try()
    return result
  }

  static exit(n) {
    f_exit(n)
    Fiber.suspend()
  }

  static exit() {
    exit(0)
  }
}

class Window {
  foreign static title=(value)
  foreign static title
  foreign static vsync=(value)
  foreign static lockstep=(value)
  foreign static fullscreen=(value)
  foreign static fullscreen
  foreign static width
  foreign static height

  foreign static resize(width, height)
}

