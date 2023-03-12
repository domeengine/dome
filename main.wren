import "dome" for Window
import "graphics" for Canvas, Color
class Main {
  construct new() {}
  init() {
    Window.resize(1920, 1280)
  }
  update() {}
  draw(dt) {
    Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
  }
}

var Game = Main.new()
