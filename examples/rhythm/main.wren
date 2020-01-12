import "audio" for AudioData, AudioEngine

class Game {
  static init() {
    AudioEngine.load("music", "res/music.ogg")
    var channel = AudioEngine.play("music")
  }

  static update() {

  }

  static draw(alpha) {}

}
