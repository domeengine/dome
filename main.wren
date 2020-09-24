import "graphics" for Canvas, Color
import "dome" for Json

class Game {
    static init() {
      
      var json = Json.parse("{\"George\":\"Harrison\",\"Paul\":\"McCartney\",\"John\":\"Lennon\",\"Ringo\":\"Starr\"}").json
      System.print(json)
      
    }
    static update() {}
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
