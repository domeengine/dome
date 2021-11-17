import "math" for Vec
import "plugin" for Plugin
import "input" for Keyboard
import "graphics" for Canvas, Color
import "dome" for Window

Plugin.load("raycaster")
// The plugin will be initialised now

// Plugins can register their own modules
import "raycaster" for Raycaster, WorldTile
var SPEED = 0.1
var R_SPEED = 0.75

class Main {
  construct new() {}

  init() {
    var SCALE = 2
    var C_SCALE = 2
    Canvas.resize(C_SCALE * 320, C_SCALE* 200)
    Window.resize(SCALE*Canvas.width, SCALE*Canvas.height)
    Window.lockstep = true

    _pos = Vec.new(4, 4)
    _angle = 180
    computeDirection()

    // and allocators for foreign classes
    _raycaster = Raycaster.init()
    _raycaster.setDimensions(14, 14)
    _raycaster.setPosition(_pos.x, _pos.y)
    _raycaster.setAngle(_angle)
    _raycaster.loadTexture("res/wall1.png")
    _raycaster.loadTexture("res/wall2.png")
    _raycaster.loadTexture("res/wall3.png")
    _raycaster.loadTexture("res/wall4.png")
    _raycaster.loadTexture("res/blankwall.png")
    _raycaster.loadTexture("res/blankwall.png")
    _raycaster.loadTexture("res/floor.png")
    _raycaster.loadTexture("res/ceil.png")
    _raycaster.loadTexture("res/guard-test.png")
    for (y in 0...14) {
      for (x in 0...14) {
        if (x == 0 || y == 0 || x == 13 || y == 13) {
           _raycaster.tile(x, y).solid(true).wallTextureId(1)
        } else {
          _raycaster.tile(x, y).solid(false).floorTextureId(7).ceilingTextureId(8)
        }
      }
    }


    _tile = WorldTile.init(_raycaster, 8, 8)
    _tile.state = 1
    _tile.mode = 1

    _raycaster.pushObject(12, 12, 9)
    _raycaster.pushObject(12, 11, 9)
    _obj = _raycaster.pushObject(10, 11, 9)
    _obj.remove()

    _raycaster.tile(6, 6).thin(true).offset(-0.5)
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

    _tile.state(_tile.state + _tile.mode * 0.05)
    if (_tile.state <= 0 || _tile.state >= 1) {
      _tile.mode(-1 * _tile.mode)
      _tile.state(_tile.mode < 0 ? 1 : 0)
    }

    _raycaster.update()
  }
  draw(alpha) {
    Canvas.cls()
    Canvas.rectfill(0, 0, Canvas.width, Canvas.height / 2, Color.lightgray)
    Canvas.rectfill(0, Canvas.height / 2, Canvas.width, Canvas.height / 2, Color.darkgray)
    _raycaster.draw(alpha)
  }
}


var Game = Main.new()
