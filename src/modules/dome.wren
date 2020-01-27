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

  foreign static resize(width, height)
  foreign static enableFullscreen()
  foreign static disableFullscreen()
}

