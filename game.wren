import "input" for Keyboard
import "graphics" for Canvas, Color, ImageData

class Game {

  static init() {
    __x = 10
    __y = 10
    __w = 5
    __h = 5
    __t = 0

    __ship = [ImageData.loadFromFile("img/ship1.png"), ImageData.loadFromFile("img/ship2.png")]

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
    __t = __t + 1
  }

  static draw(dt) {
    Canvas.cls()
    var color = Color.new(171, 82, 54).rgb
    Canvas.rectfill(__x, __y, __w, __h, color)


    // Proposals for drawing an image
    var frame = (__t / 5).floor % 2
    Canvas.draw(__ship[frame], __x, __y)
    // __image.draw(__x, __y)

  }
}
