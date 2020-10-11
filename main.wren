import "graphics" for Canvas, Color
import "code" for Code 

class Game {
    static init() {
      Code.init()
    }
    static update() {}
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
