import "plugin" for Plugin

Plugin.load("test")
// The plugin will be initialised now

// Plugins can register their own modules
import "external" for ExternalClass


class Main {
  construct new() {}

  init() {
    // and allocators for foreign classes
    var obj = ExternalClass.init()

    // and finally, they can register foreign methods implemented
    // in the plugin native language.
    obj.alert("Some words")
  }

  update() {}
  draw(dt) {}
}

var Game = Main.new()
