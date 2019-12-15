class Process {
  foreign static f_exit(n)
  static exit(n) { f_exit(n) }
  static exit() {
    exit(0)
  }
}

