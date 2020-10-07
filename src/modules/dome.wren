class Dependency {
  
  version {_version}
  name {_name}

  construct new(name, version) {
    _name = name
    _version = version
  }

  major { toList[0] }
  minor { toList[1] }
  patch { toList[2] }

  toString {{"name":name, "version":version}.toString}

  toList {
    if (!_list) {
      _list = version.split(".").map {|value| Num.fromString(value) }.toList
    }
    return _list
  }

  atLeast(version) {
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

class Version {
  foreign static toString

  static dome  { Dependency.new("dome", Version.toString) }
  static wren  { Dependency.new("wren", "3.0.0") }

  static deps  {{
    "dome": Version.dome,
    "wren": Version.wren
  }}

  static major { Version.dome.major }
  static minor { Version.dome.minor }
  static patch { Version.dome.patch }

  static toList {Version.dome.toList}
  static atLeast (version) {
    return Version.dome.atLeast(version)
  }
}

class Process {
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

  foreign static resize(width, height)
}

