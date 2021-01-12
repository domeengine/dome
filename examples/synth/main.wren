import "graphics" for Canvas, Color
import "dome" for Window
import "plugin" for Plugin
import "input" for Keyboard

Plugin.load("synth")
import "synth" for Synth



var KEYS = [
  "`","z","s","x","c","f","v","g","b","h","n","m", ",", "."
]

class Game {
    static init() {
      __note = 0
      __octave = 4
    }
    static update() {
      if (Keyboard["up"].justPressed) {
        __octave = __octave + 1
      }
      if (Keyboard["down"].justPressed) {
        __octave = __octave - 1
      }
      if (Keyboard["left"].justPressed) {
        Synth.volume = Synth.volume - 0.05
      }
      if (Keyboard["right"].justPressed) {
        Synth.volume = Synth.volume + 0.05
      }
      for (i in -1...13) {
        var name = KEYS[i+1]
        if (Keyboard[name].justPressed) {
          __note = i
          Synth.noteOn(__octave, __note)
        } else if (!Keyboard[name].down && Keyboard[name].previous) {
          Synth.noteOff()
        }
      }
      if (Keyboard["1"].justPressed) {
        Synth.storePattern("16c1 32#g3 a #a b c 64_ #c d #d e f #f g #g")
        Synth.playPattern()
      }
    }

    static draw(dt) {
      Canvas.cls()
    }
}
