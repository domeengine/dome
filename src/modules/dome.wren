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
  foreign static args
  foreign static f_exit(n)
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
  foreign static fps

  foreign static resize(width, height)
}

// Private string handling methods
// Do not use for game code.
class StringUtils {
  foreign static toLowercase(string)

  static subString(str, start, len) {
    return str.bytes.skip(start).take(len).toList
  }
}
