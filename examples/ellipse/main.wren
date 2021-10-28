import "graphics" for Canvas, Color
import "input" for Keyboard

var T = Color.rgb(255, 255, 255, 128)

class Main {
  construct new() {}

  init() {
    _sizeX = 0
    _sizeY = 0
  }

  update() {
    if (Keyboard["left"].down) {
      _sizeX = _sizeX - 1
    }
    if (Keyboard["right"].down) {
      _sizeX = _sizeX + 1
    }
    if (Keyboard["up"].down) {
      _sizeY = _sizeY + 1
    }
    if (Keyboard["down"].down) {
      _sizeY = _sizeY - 1
    }
  }

  draw(dt) {
    Canvas.cls()
    var cX = Canvas.width / 2
    var cY = Canvas.height / 2
    var x1 = cX - _sizeX
    var x2 = cX + _sizeX
    var y1 = cY - _sizeY
    var y2 = cY + _sizeY
    Canvas.ellipsefill(x1, y1, x2, y2, Color.green)
    Canvas.ellipse(x1, y1, x2, y2, T)
    Canvas.pset(cX, cY, Color.blue)
  }
}

var Game = Main.new()
