import "vector" for Vector
import "dome" for StringUtils

class Key {
  construct init() {
    _down = false
    _current = false
    _previous = false
    _halfTransitions = 0
    _repeats = 0
  }
  commit() {
    _previous = _down
    _down = _current
    if (_down && _previous == _down) {
      _repeats = _repeats + 1
    } else {
      _repeats = 0
    }
  }

  update(state) {
    _current = state
  }

  down { _down }
  previous { _previous }
  repeats { _repeats }
}

class Keyboard {
  static isKeyDown(key) {
    return Keyboard[StringUtils.toLowercase(key)].down
  }
  static init() {
    __keys = {}
    Keyboard.f_captureVariable()
  }

  static [name] {
    name = StringUtils.toLowercase(name)
    if (!__keys.containsKey(name)) {
      update(name, false)
    }
    return __keys[name]
  }

  // PRIVATE, called by game loop
  static update(keyName, state) {
    if (!__keys.containsKey(keyName)) {
      __keys[keyName] = Key.init()
    }
    __keys[keyName].update(state)
  }

  static commit() {
    __keys.values.each {|key| key.commit() }
  }

  foreign static f_captureVariable()
}
Keyboard.init()


//# Keyboard.isKeyDown(keyname)
//# Keyboard[keyname].down
//# Keyboard[keyname].up
//# Keyboard[keyname].repeats

class Mouse {
  foreign static x
  foreign static y
  foreign static isButtonPressed(key)

  foreign static hidden
  foreign static hidden=(value)
}

foreign class GamePad {

  construct open(index) {}
  foreign close()

  foreign attached
  foreign id
  foreign name
  isButtonPressed(key) {
    return f_isButtonPressed(key)
  }

  foreign f_isButtonPressed(key)
  foreign f_getAnalogStick(side)
  foreign getTrigger(side)

  getAnalogStick(side) {
    var stick = f_getAnalogStick(side)
    return Vector.new(stick[0], stick[1])
  }

  static init_() {
    __pads = {}
    __dummy = GamePad.open(-1)
    f_getGamePadIds().each {|id|
      addGamePad(id)
    }
  }

  static [n] {
    if (!__pads[n]) {
      return __dummy
    }
    return __pads[n]
  }

  static all { __pads.values }
  static next {
    if (__pads.count > 0) {
      return __pads.values.where {|pad| pad.attached }.toList[0]
    } else {
      return __dummy
    }
  }

  static addGamePad(joystickId) {
    var pad = GamePad.open(joystickId)
    __pads[pad.id] = pad
  }

  static removeGamePad(instanceId) {
    __pads[instanceId].close()
    __pads.remove(instanceId)
  }

  foreign static f_getGamePadIds()
}

GamePad.init_()

