import "graphics" for Canvas, Color
import "input" for Keyboard

var T = Color.rgb(255, 255, 255, 128)

class Game {
    static init() {
      __size = 0
    }
    static update() {
      if (Keyboard["space"].justPressed) {
        __size = __size + 1
      }


    }
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
      Canvas.rectfill(0, 0, __size, __size, Color.green)
      Canvas.rectfill(0, 0, __size, __size, T)
      Canvas.rect(0, 0, __size, __size, Color.red)
      Canvas.pset(0, 0, Color.blue)
      Canvas.pset(__size - 1, 0, Color.blue)
      Canvas.pset(0, __size - 1, Color.blue)
      Canvas.pset(__size - 1, __size - 1, Color.blue)
    }
}
