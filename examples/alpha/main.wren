import "graphics" for Canvas, Color
var I = 255
var D = -1
class Game {
    static init() {}
    static update() {
      I = I + D
      if (I <= 0 || I >= 255) {
        D = -D
      }
    }
    static draw(dt) {
      Canvas.cls()
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
      Canvas.print(I, 0, 0, Color.white)
      Canvas.rectfill(10, 40, 50, 50, Color.rgb(0,255,0))
      Canvas.rectfill(10, 20, 50, 50, Color.rgb(255,0,0,I))
    }
}
