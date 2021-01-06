import "graphics" for Canvas, Color
import "audio" for AudioEngine
import "input" for Keyboard
class Game {
    static init() {
      AudioEngine.load("music", "../spaceshooter/res/around-the-corner.ogg")
      AudioEngine.load("sfx", "../spaceshooter/res/Laser_Shoot.wav")
      __channel = AudioEngine.play("music")

    }
    static update() {
      if (Keyboard["return"].justPressed) {
        AudioEngine.play("sfx")
      }
      if (Keyboard["space"].justPressed) {
        __channel = AudioEngine.play("music")
      }
      if (Keyboard["backspace"].justPressed) {
        __channel.stop()
      }
      if (Keyboard["left"].justPressed) {
        __channel.pan = __channel.pan - 0.05
      }
      if (Keyboard["right"].justPressed) {
        __channel.pan = __channel.pan + 0.05
      }
    }
    static draw(dt) {
      Canvas.cls()
      Canvas.print(__channel == null ? "Nothing Playing" : "Music Playing", 10, 10, Color.white)
      Canvas.print(__channel == null ? "" : "Volume %(__channel.volume)", 10, 18, Color.white)
      Canvas.print(__channel == null ? "" : "Pan %(__channel.pan)", 10, 26, Color.white)
    }
}
