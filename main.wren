import "graphics" for Canvas, Color
import "dome" for Process
class Game {
    static init() {
      System.print(Process.args)
    }
    static update() {}
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
