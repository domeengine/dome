import "input" for Keyboard
import "graphics" for Graphics, Color


class Game {

  static init() {
    __x = 10
    __y = 10
    __w = 5
    __h = 5
  }

  static update(dt) {
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
    for (i in 0...Graphics.screenWidth) {
      for (j in 0...Graphics.screenHeight) {
        Graphics.pset(i, j, Color.rgb(0,0,0,0))
      }
    }
    // Fillrect
    for (i in 0...__w) {
      for (j in 0...__h) {
        Graphics.pset(__x+i, __y+j, Color.rgb(255,255,255,0))
      }
    }
  }
}
