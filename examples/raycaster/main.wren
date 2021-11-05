import "plugin" for Plugin

Plugin.load("raycaster")
// The plugin will be initialised now

// Plugins can register their own modules
import "raycaster" for Raycaster


class Main {
  construct new() {}

  init() {
    // and allocators for foreign classes
    _raycaster = Raycaster.init()
  }

  update() {}
  draw(alpha) {
    _raycaster.draw(alpha)
  }
}

var Game = Main.new()
