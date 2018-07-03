import "input" for Keyboard
import "graphics" for Graphics, Color

class Game {

  static init() {
    __x = 10
    __y = 10
    __w = 5
    __h = 5
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

    // Fillrect
  }
  static draw(dt) {
    Graphics.cls()
    // Fillrect
    var color = Color.new(171, 82, 54).rgb
    Graphics.rectfill(__x, __y, __w, __h, color)
    /*
    for (i in 0...__w) {
      for (j in 0...__h) {
        Graphics.pset(__x+i, __y+j, color)
      }
    }
    */
  }
}
