import "graphics" for Canvas, Color
import "input" for Keyboard
import "dome" for Window
var X = 50
var Y = 50

class Game {
    static init() {
      __text = ""
      // Window.resize(Canvas.width, Canvas.height)
      Keyboard.handleText = true
      Keyboard.textRegion(X, Y, 8, 8)
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
      Canvas.rect(X, Y, 8, 8, Color.red)
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
      Canvas.print(__text, 10, 20, Color.white)
    }
}
