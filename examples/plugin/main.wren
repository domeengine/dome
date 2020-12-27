import "graphics" for Canvas, Color
import "dome" for Window
import "plugin" for Plugin

Plugin.load("test")
import "external" for Test

class Game {
    static init() {
      Test.begin()
      Test.end()
    }
    static update() {
    }
    static draw(dt) {
      Canvas.cls()
      Canvas.print(Test.value, 10, 10, Color.white)
    }
}
