import "graphics" for Canvas, Color
import "input" for Keyboard

var T = Color.rgb(255, 255, 255, 128)

class Main {
  construct new() {}

  init() {
    _size = 0
  }

  update() {
    if (Keyboard["space"].justPressed) {
      _size = _size + 1
    }
  }

  draw(dt) {
    Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    Canvas.rectfill(0, 0, _size, _size, Color.green)
    Canvas.rectfill(0, 0, _size, _size, T)
    Canvas.rect(0, 0, _size, _size, Color.red)
    Canvas.pset(0, 0, Color.blue)
    Canvas.pset(_size - 1, 0, Color.blue)
    Canvas.pset(0, _size - 1, Color.blue)
    Canvas.pset(_size - 1, _size - 1, Color.blue)
  }
}

var Game = Main.new()
