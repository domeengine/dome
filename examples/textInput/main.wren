import "graphics" for Canvas, Color
import "input" for Keyboard

class Game {
    static init() {
      __text = ""
      Keyboard.handleText = true
    }

    static update() {
      __text = __text + Keyboard.text
      if (Keyboard["backspace"].justPressed && __text.count > 0) {
        var codePoints = __text.codePoints
        codePoints = codePoints.take(codePoints.count - 1)
        __text = ""
        for (point in codePoints) {
          __text = __text + String.fromCodePoint(point)
        }
      }
      System.print(__text)
    }

    static draw(dt) {
      Canvas.cls()
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
      Canvas.print(__text, 10, 20, Color.white)
    }
}
