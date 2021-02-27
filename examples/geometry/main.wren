import "dome" for Window
import "math" for M
import "graphics" for Canvas, Color

var CENTER_X = 200
var CENTER_Y = 200

var RADIUS = 200

class Main {
  construct new() {}

  init() {
    Window.resize(400, 400)
    Canvas.resize(400, 400)
    _angle = 0
  }

  update() {
    _angle = (_angle + (Num.pi / 180)) % (2 * Num.pi)
  }

  draw(dt) {
    Canvas.cls()
    var x = CENTER_X + M.cos(_angle) * RADIUS
    var y = CENTER_Y + M.sin(_angle) * RADIUS
    Canvas.line(CENTER_X, CENTER_Y, x, y, Color.blue, 6)
    Canvas.print("%(x)", 1, 1, Color.red)
    Canvas.print("%(y)", 1, 10, Color.red)
    Canvas.print("%(_angle)", 1, 18, Color.red)
  }
}

var Game = Main.new()
