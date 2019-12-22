class Process {
  foreign static f_exit(n)
  static exit(n) { f_exit(n) }
  static exit() {
    exit(0)
  }
}

class Window {
  foreign static resize(width, height)
  foreign static title=(value)
}

