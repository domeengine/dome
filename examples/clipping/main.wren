import "input" for Keyboard
import "graphics" for Canvas, Color

var C = Color.rgb(171, 82, 54)

class Main {
  construct new() {}

  init() {
    _x = 10
    _y = 10
    _w = 5
    _h = 5

    _cx = 0
    _cy = 0
  }

  update() {
    if (Keyboard.isKeyDown("left")) {
      _x = _x - 1
    }
    if (Keyboard.isKeyDown("right")) {
      _x = _x + 1
    }
    if (Keyboard.isKeyDown("up")) {
      _y = _y - 1
    }
    if (Keyboard.isKeyDown("down")) {
      _y = _y + 1
    }
    if (Keyboard.isKeyDown("a")) {
      _cx = _cx - 1
    }
    if (Keyboard.isKeyDown("d")) {
      _cx = _cx + 1
    }
    if (Keyboard.isKeyDown("w")) {
      _cy = _cy - 1
    }
    if (Keyboard.isKeyDown("s")) {
      _cy = _cy + 1
    }
  }

  draw(alpha) {
    Canvas.cls()
    Canvas.offset(_cx, _cy)
    Canvas.clip(-10, -10, Canvas.width / 4, Canvas.height / 4)
    Canvas.rectfill(0, 0, Canvas.width / 2, Canvas.height / 2, Color.white)

    Canvas.rectfill(_x, _y, _w, _h, C)
    Canvas.print("Hello world", (Canvas.width - 8 * 11) / 2, Canvas.height / 2, Color.red)
    Canvas.clip()
    Canvas.print("Hello world", 0, 0, Color.white)
  }
}

var Game = Main.new()
