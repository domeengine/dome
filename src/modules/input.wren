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
  foreign isButtonPressed(key)
  foreign f_getAnalogStick(side)
  foreign getTrigger(side)
  getAnalogStick(side) {
    var stick = f_getAnalogStick(side)
    return Vector.new(stick[0], stick[1])
  }

  static s_initialise() {
    System.print("Gamepad activation")

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


GamePad.s_initialise()
System.print(GamePad[0].isButtonPressed("X"))


