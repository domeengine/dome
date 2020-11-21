import "input" for Keyboard
import "graphics" for Canvas, Color

var C = Color.rgb(171, 82, 54)

class Game {

  static init() {
    __x = 10
    __y = 10
    __w = 5
    __h = 5

    __cx = 0
    __cy = 0
  }

  static update() {
    if (Keyboard.isKeyDown("left")) {
      __x = __x - 1
    }
    if (Keyboard.isKeyDown("right")) {
      __x = __x+ 1
    }
    if (Keyboard.isKeyDown("up")) {
      __y = __y - 1
    }
    if (Keyboard.isKeyDown("down")) {
      __y = __y + 1
    }
    if (Keyboard.isKeyDown("a")) {
      __cx = __cx - 1
    }
    if (Keyboard.isKeyDown("d")) {
      __cx = __cx+ 1
    }
    if (Keyboard.isKeyDown("w")) {
      __cy = __cy - 1
    }
    if (Keyboard.isKeyDown("s")) {
      __cy = __cy + 1
    }
  }
  static draw(alpha) {
    Canvas.cls()
    Canvas.offset(__cx, __cy)
    Canvas.clip(-10, -10, Canvas.width / 4, Canvas.height / 4)
    Canvas.rectfill(0, 0, Canvas.width / 2, Canvas.height / 2, Color.white)

    Canvas.rectfill(__x, __y, __w, __h, C)
    Canvas.print("Hello world", (Canvas.width - 8 * 11) / 2, Canvas.height / 2, Color.red)
    Canvas.clip()
    Canvas.print("Hello world", 0, 0, Color.white)
  }
}
