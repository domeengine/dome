import "graphics" for Canvas, Color
import "input" for Keyboard

var T = Color.rgb(255, 255, 255, 128)

class Main {
  construct new() {}

  init() {
    _size = 0
  }

  update() {
    if (Keyboard["space"].down) {
      _size = _size + 1
    }
    if (Keyboard["up"].down) {
      _size = _size + 1
    }
    if (Keyboard["down"].down) {
      _size = _size - 1
    }
  }

  draw(dt) {
    Canvas.cls()
    var cX = Canvas.width / 2
    var cY = Canvas.height / 2
    Canvas.circlefill(cX, cY, _size, Color.green)
    Canvas.circle(cX, cY, _size, T)
    Canvas.pset(cX, cY, Color.blue)
  }
}

var Game = Main.new()
