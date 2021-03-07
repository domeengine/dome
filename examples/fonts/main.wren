import "graphics" for Canvas, Color
import "font" for Font

class Main {
  construct new() {}

  init() {
    _defaultPos = Canvas.getPrintArea("Hello world")

    Font.load("memory", "memory.ttf", 50)
    Font.load("memory_small", "memory.ttf", 16)
    Font["memory"].antialias = true
    _pos = Font["memory_small"].getArea("Hello\nworld")
    Canvas.font = "memory"
  }

  update() {}

  draw(dt) {
    Canvas.cls()
    Canvas.rect(0, 0, _pos.x, _pos.y, Color.red)

    Canvas.print("Hello\nworld", 0, 0, Color.hsv(80, 1, 1), "memory_small")
    Canvas.print("The quick brown fox jumps over the lazy dog", 0, 60, Color.hsv(80, 1, 1), "memory_small")
    var pos = Canvas.getPrintArea("DOME Installed\nSuccessfully.")
    Canvas.rect(10, 80, pos.x, pos.y, Color.red)
    Canvas.print("DOME Installed\nSuccessfully.", 10, 80, Color.white)

    Canvas.rect(10, 160, _defaultPos.x, _defaultPos.y, Color.red)
    Canvas.print("Hello\nworld", 10, 160, Color.white, Font.default)
  }
}

var Game = Main.new()
