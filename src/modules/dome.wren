import "color" for Color
import "platform" for Platform
import "collections" for Stack
import "stringUtils" for StringUtils

class Log {
  static init() {
    __stack = Stack.new()
    f_level = "INFO"
  }
  static context { __stack }
  static print(level, text) {
    var context = __stack.peek()
    print(level, text, context)
  }

  foreign static f_level=(v)
  static level=(v) { f_level = StringUtils.toUppercase(v) }
  foreign static level
  foreign static print(level, text, context)

  static debug(text) {
    print(5, text)
  }
  static d(text) {
    print(5, text)
  }

  static info(text) {
    print(4, text)
  }
  static i(text) {
    print(4, text)
  }

  static warn(text) {
    print(3, text)
  }
  static w(text) {
    print(3, text)
  }

  static error(text) {
    print(2, text)
  }
  static e(text) {
    print(2, text)
  }

  static fatal(text) {
    print(1, text)
  }
  static f(text) {
    print(1, text)
  }
}
Log.init()

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
  foreign static errorDialog=(value)
  foreign static errorDialog
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
  foreign static integerScale
  foreign static integerScale=(value)

  foreign static f_color=(value)
  foreign static f_color
  static color=(value) {
    var c = Color.black
    if (value is Color) {
      c = value
    }
    f_color = c.toNum
  }
  static color { Color.fromNum(f_color) }

  foreign static resize(width, height)
}
