import "graphics" for Canvas, Color
import "dome" for Window
import "math" for Vec

var Background = Color.rgb(200, 0, 0, 128)

class Main {
  construct new() {}
  init() {
    Canvas.resize(200, 200)
    Window.resize(Canvas.width * 2, Canvas.height * 2)
    _center = Vec.new(Canvas.width / 2, Canvas.height / 2)

    _length = 40
    _start = 0
    var angle = Num.pi * 0 / 180
    _p0 = _center + Vec.new(angle.cos, angle.sin) * _length
    angle = angle + Num.pi * 120 / 180
    _p1 = _center + Vec.new(angle.cos, angle.sin) * _length
    angle = angle + Num.pi * 120 / 180
    _p2 = _center + Vec.new(angle.cos, angle.sin) * _length
  }
  update() {
    _start = _start + 1
    var angle = Num.pi * _start / 180
    _p0 = _center + Vec.new(angle.cos, angle.sin) * _length
    angle = angle + Num.pi * 120 / 180
    _p1 = _center + Vec.new(angle.cos, angle.sin) * _length
    angle = angle + Num.pi * 120 / 180
    _p2 = _center + Vec.new(angle.cos, angle.sin) * _length

  }
  draw(dt) {
    Canvas.cls()
    Canvas.trianglefill(_p0.x, _p0.y, _p1.x, _p1.y, _p2.x, _p2.y, Background)
    Canvas.triangle(_p0.x, _p0.y, _p1.x, _p1.y, _p2.x, _p2.y, Color.yellow)
  }
}

var Game = Main.new()
