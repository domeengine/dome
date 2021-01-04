import "graphics" for Canvas, Color
import "audio" for AudioEngine
import "input" for Keyboard
class Game {
    static init() {
      AudioEngine.load("music", "../spaceshooter/res/around-the-corner.ogg")
      AudioEngine.load("sfx", "../spaceshooter/res/Laser_Shoot.wav")
      AudioEngine.play("music")

    }
    static update() {
      if (Keyboard["space"].justPressed) {
        AudioEngine.play("sfx")
      }
    }
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
