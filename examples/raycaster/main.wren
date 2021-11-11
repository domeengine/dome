import "math" for Vec
import "plugin" for Plugin
import "input" for Keyboard
import "graphics" for Canvas, Color
import "dome" for Window

Plugin.load("raycaster")
// The plugin will be initialised now

// Plugins can register their own modules
import "raycaster" for Raycaster
var SPEED = 0.1
var R_SPEED = 0.75

class Main {
  construct new() {}

  init() {
    var SCALE = 3
    Canvas.resize(320, 200)
    Window.resize(SCALE*Canvas.width, SCALE*Canvas.height)
    Window.lockstep = true

    _pos = Vec.new(22, 12)
    _angle = 180
    computeDirection()

    // and allocators for foreign classes
    _raycaster = Raycaster.init()
    _raycaster.setPosition(_pos.x, _pos.y)
    _raycaster.setAngle(_angle)
    _raycaster.loadTexture("res/wall1.png")
    _raycaster.loadTexture("res/wall2.png")
    _raycaster.loadTexture("res/wall3.png")
    _raycaster.loadTexture("res/wall4.png")
  }

  computeDirection() {
    var rads = _angle * Num.pi / 180
    _dir = Vec.new(rads.cos, rads.sin)
    _perp = Vec.new(-_dir.y, _dir.x)
  }

  update() {
    // TODO: normalise movement
    if (Keyboard["a"].down) {
      _pos = _pos - _perp * SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["d"].down) {
      _pos = _pos + _perp * SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["w"].down || Keyboard["up"].down) {
      _pos = _pos + _dir*SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["s"].down || Keyboard["down"].down) {
      _pos = _pos - _dir*SPEED
      _raycaster.setPosition(_pos.x, _pos.y)
    }
    if (Keyboard["left"].down) {
      _angle = _angle - R_SPEED
      computeDirection()
      _raycaster.setAngle(_angle)
    }
    if (Keyboard["right"].down) {
      _angle = _angle + R_SPEED
      computeDirection()
      _raycaster.setAngle(_angle)
    }
  }
  draw(alpha) {
    Canvas.cls()
    Canvas.rectfill(0, 0, Canvas.width, Canvas.height / 2, Color.lightgray)
    Canvas.rectfill(0, Canvas.height / 2, Canvas.width, Canvas.height / 2, Color.darkgray)
    _raycaster.draw(alpha)
  }
}

var Game = Main.new()
