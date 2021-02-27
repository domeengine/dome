import "graphics" for Canvas, Color
import "audio" for AudioEngine
import "input" for Keyboard

class Main {
  construct new() {}

  init() {
    AudioEngine.load("music", "../spaceshooter/res/around-the-corner.ogg")
    AudioEngine.load("sfx", "../spaceshooter/res/Laser_Shoot.wav")
    _channel = AudioEngine.play("music")
  }

  update() {
    if (Keyboard["return"].justPressed) {
      AudioEngine.play("sfx")
    }
    if (Keyboard["space"].justPressed) {
      _channel = AudioEngine.play("music")
    }
    if (Keyboard["backspace"].justPressed) {
      _channel.stop()
    }
    if (Keyboard["0"].justPressed) {
      AudioEngine.stopAllChannels()
    }
    if (Keyboard["-"].justPressed) {
      AudioEngine.unload("music")
      AudioEngine.unload("sfx")
    }
    if (Keyboard["left"].down) {
      _channel.pan = _channel.pan - 0.01
    }
    if (Keyboard["right"].down) {
      _channel.pan = _channel.pan + 0.01
    }
    if (Keyboard["down"].down) {
      _channel.volume = _channel.volume - 0.01
    }
    if (Keyboard["up"].down) {
      _channel.volume = _channel.volume + 0.01
    }
    if (_channel.pan.abs < 0.009) {
      _channel.pan = 0
    }
    if (_channel.volume.abs < 0.009) {
      _channel.volume = 0
    }
  }

  draw(dt) {
    Canvas.cls()
    Canvas.print(_channel == null ? "Nothing Playing" : "Music Playing", 10, 10, Color.white)
    Canvas.print(_channel == null ? "" : "Volume %(_channel.volume)", 10, 18, Color.white)
    Canvas.print(_channel == null ? "" : "Pan %(_channel.pan)", 10, 26, Color.white)
  }
}

var Game = Main.new()

