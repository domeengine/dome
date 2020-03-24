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

  foreign static resize(width, height)
}

class Version {
  foreign static name
  foreign static hash
  foreign static number

  static print() {
    System.print("Debug: DOME Version %(name) | %(number) | %(hash)")
  }

  static isAtLeast(version) {
    if(version > number) {
      System.print("Error: DOME Version %(version) or greater required. Current %(name) (%(number))")
      Process.exit(-1)
    }
  }
}