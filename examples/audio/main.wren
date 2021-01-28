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
      if (Keyboard["0"].justPressed) {
        AudioEngine.stopAllChannels()
      }
      if (Keyboard["-"].justPressed) {
        AudioEngine.unload("music")
        AudioEngine.unload("sfx")
      }
      if (Keyboard["left"].down) {
        __channel.pan = __channel.pan - 0.01
      }
      if (Keyboard["right"].down) {
        __channel.pan = __channel.pan + 0.01
      }
      if (Keyboard["down"].down) {
        __channel.volume = __channel.volume - 0.01
      }
      if (Keyboard["up"].down) {
        __channel.volume = __channel.volume + 0.01
      }
      if (__channel.pan.abs < 0.009) {
        __channel.pan = 0
      }
      if (__channel.volume.abs < 0.009) {
        __channel.volume = 0
      }
    }
    static draw(dt) {
      Canvas.cls()
      Canvas.print(__channel == null ? "Nothing Playing" : "Music Playing", 10, 10, Color.white)
      Canvas.print(__channel == null ? "" : "Volume %(__channel.volume)", 10, 18, Color.white)
      Canvas.print(__channel == null ? "" : "Pan %(__channel.pan)", 10, 26, Color.white)
    }
}
