import "graphics" for Canvas, Color, AsepriteImage
import "dome" for Window

class Main {
  construct new() {}
  init() {
    Canvas.resize(640, 512)
    Window.resize(Canvas.width,Canvas.height)
    _i = 0
    _ase = AsepriteImage.loadFromFile("test.ase")
    System.print(_ase.layers)
  }
  update() {
    _i = _i + 1
  }
  draw(dt) {
    Canvas.cls()
    Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    _ase.draw(_i / 60, null)
  }
}

var Game = Main.new()
