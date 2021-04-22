import "graphics" for Canvas, Color
import "input" for Keyboard, Clipboard
import "dome" for Window

var X = 10
var Y = 28

class Game {
    static init() {
      __text = ""
      Keyboard.handleText = true
      Keyboard.textRegion(X, Y, 8, 8)
    }

    static update() {
      var change = false
      if (Keyboard.text.count > 0) {
        __text = __text + Keyboard.text
        change = true
      }

      if (!Keyboard.compositionText && Keyboard["backspace"].justPressed && __text.count > 0) {
        var codePoints = __text.codePoints
        codePoints = codePoints.take(codePoints.count - 1)
        __text = ""
        for (point in codePoints) {
          __text = __text + String.fromCodePoint(point)
        }
        change = true
      }
      if (change) {
        Keyboard.textRegion(__text.count * 8, Y, 8, 8)
      }

      if ((Keyboard["left ctrl"].down || Keyboard["right ctrl"].down) && Keyboard["c"].justPressed) {
        Clipboard.content = __text
      }
      if ((Keyboard["left ctrl"].down || Keyboard["right ctrl"].down) && Keyboard["v"].justPressed) {
        __text = __text + Clipboard.content
      }
    }

    static draw(dt) {
      Canvas.cls()
      Canvas.rect(X, Y, 8, 8, Color.red)
      Canvas.print("Enter Text:", 10, 10, Color.white)
      Canvas.print(__text, 10, 20, Color.white)
      if (Keyboard.compositionText) {
        var left = 10 + 8 * __text.count
        Canvas.print(Keyboard.compositionText, left, 20, Color.red)
        var range = Keyboard.compositionRange
        var length = range.max - range.min
        if (length > 0) {
          Canvas.line(left + 8 * range.min, 30, left + 8 * (range.min + length), 30, Color.red)
        } else {
          Canvas.line(left, 30, left + 8 * (Keyboard.compositionText.count), 30, Color.red)
        }
      }
    }
}
