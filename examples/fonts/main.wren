import "graphics" for Canvas, Color
import "font" for Font

class Game {
    static init() {
      Font.load("memory", "memory.ttf", 50)
      Font.load("memory_small", "memory.ttf", 16)
      Font["memory"].antialias = true
      Canvas.font = "memory"
    }
    static update() {}
    static draw(dt) {
      Canvas.cls()
      Canvas.print("The quick brown fox jumps over the lazy dog", 0, 120, Color.hsv(80, 1, 1), "memory_small")
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
