import "vector" for Vector

class Keyboard {
  foreign static isKeyDown(key)
}

class Mouse {
  foreign static x
  foreign static y
  foreign static isButtonPressed(key)
}

foreign class GamePad {
  construct open(index) {}
  foreign attached
  foreign name
  foreign f_isButtonPressed(key)
  isButtonPressed(key) {
    return f_isButtonPressed(key)
  }

  foreign f_getAnalogStick(side)
  foreign getTrigger(side)
  getAnalogStick(side) {
    var stick = f_getAnalogStick(side)
    return Vector.new(stick[0], stick[1])
  }

  foreign static f_getGamePadIds()
  static discover() {
    return f_getGamePadIds().map {|id|
      GamePad[id]
      return id
    }
  }

  static [n] {
    if (!__pads) {
      __pads = {}
    }
    if (!__pads[n]) {
      __pads[n] = GamePad.open(n)
      System.print(n)
    }
    return __pads[n]
  }
}

GamePad.discover()

