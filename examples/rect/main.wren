import "graphics" for Canvas, Color
import "input" for Keyboard
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
      Canvas.rect(0, 0, __size, __size, Color.red)
      Canvas.pset(0, 0, Color.blue)
      Canvas.pset(__size, 0, Color.blue)
      Canvas.pset(0, __size, Color.blue)
      Canvas.pset(__size, __size, Color.blue)
    }
}
