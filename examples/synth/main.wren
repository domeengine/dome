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
      Synth.storePattern("16g2 16f2 8a1 8b1 16e2 16d2 8f1 8g1 16d2 16c2 8e1 8g1 4.c2 8- 4-")
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
      if (Keyboard["return"].justPressed) {
        Synth.playTone(350, 1000)
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
        Synth.playPattern()
      }
    }

    static draw(dt) {
      Canvas.cls()
    }
}
