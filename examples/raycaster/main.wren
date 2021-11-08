import "math" for Vec
import "plugin" for Plugin
import "input" for Keyboard
import "graphics" for Canvas
import "dome" for Process, Window

Plugin.load("raycaster")
// The plugin will be initialised now

// Plugins can register their own modules
import "raycaster" for Raycaster


var SPEED = 0.1

class Main {
  construct new() {}

  compute() {
    var rads = _angle * Num.pi / 180
    _dir = Vec.new(rads.cos, rads.sin)
  }

  init() {
    Window.lockstep = true
    _pos = Vec.new(22, 12)
    _angle = 180
    compute()

    // and allocators for foreign classes
    _raycaster = Raycaster.init()
    _raycaster.setPosition(_pos.x, _pos.y)
    _raycaster.setAngle(_angle)
  }

  update() {
    if (Keyboard["up"].down) {
      _pos = _pos + _dir*SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["down"].down) {
      _pos = _pos - _dir*SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["left"].down) {
      _angle = _angle - 0.5
      compute()
      _raycaster.setAngle(_angle)
    }
    if (Keyboard["right"].down) {
      _angle = _angle + 0.5
      compute()
      _raycaster.setAngle(_angle)
    }
  }
  draw(alpha) {
    Canvas.cls()
    _raycaster.draw(alpha)
  }
}

var Game = Main.new()
