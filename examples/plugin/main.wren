import "graphics" for Canvas, Color
import "dome" for Window
import "plugin" for Plugin

Plugin.load("test")
import "external" for Test

class Game {
    static init() {
      Test.begin()
    }
    static update() {
      if (Test.number / 60 > 5) {
        Test.end(true)
      }
    }
    static draw(dt) {
      Canvas.cls()
      var y = 10
      Canvas.print((Test.number / 60).floor, 10, y, Color.white)
      y = y + 8
      Canvas.print(Test.string, 10, y, Color.white)
      y = y + 8
      Canvas.print(Test.bytes, 10, y, Color.white)
      y = y + 8
      Canvas.print(Test.empty, 10, y, Color.white)
      y = y + 8
      Canvas.print(Test.boolean, 10, y, Color.white)
    }
}
