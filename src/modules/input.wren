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


