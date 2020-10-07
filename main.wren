import "graphics" for Canvas, Color
import "dome" for Version

class Game {
    static init() {
      System.print(Version.deps)
    }
    static update() {}
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
