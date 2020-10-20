import "dome" for Window
import "math" for M
import "graphics" for Canvas, Color

var CENTER_X = 200
var CENTER_Y = 200

var RADIUS = 200

class Game {
    static init() {
      Window.resize(400, 400)
      Canvas.resize(400, 400)
      __angle = 0
    }
    static update() {
      __angle = (__angle + (Num.pi / 180)) % (2 * Num.pi)
    }
    static draw(dt) {
      Canvas.cls()
      var x = CENTER_X + M.cos(__angle) * RADIUS
      var y = CENTER_Y + M.sin(__angle) * RADIUS
      Canvas.line(CENTER_X, CENTER_Y, x, y, Color.blue, 6)
      Canvas.print("%(x)", 1, 1, Color.red)
      Canvas.print("%(y)", 1, 10, Color.red)
      Canvas.print("%(__angle)", 1, 18, Color.red)
    }
}
