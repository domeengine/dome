import "vector" for Vector
import "stringUtils" for StringUtils

class Input {
  // This sets up the whole module's event loop behaviour
  foreign static f_captureVariables()
}

class DigitalInput {
  construct init() {
    _down = false
    _current = false
    _previous = false
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

  reset() {
    _down = false
    commit()
  }
  repeat() {
    commit()
  }

  down { _down }
  previous { _previous }
  repeats { _repeats }
  justPressed { _down && _repeats == 0 }
}


class Keyboard {

  static isButtonPressed(key) { isKeyDown(key) }
  static isKeyDown(key) {
    return Keyboard[StringUtils.toLowercase(key)].down
  }
  static init_() {
    __keys = {}
  }

  static [name] {
    name = StringUtils.toLowercase(name)
    if (!__keys.containsKey(name)) {
      update(name, false)
    }
    return __keys[name]
  }

  static allPressed {
    var pressed = {}
    __keys.keys.where {|key| __keys[key].down }.each {|key| pressed[key] = __keys[key] }
    return pressed
  }

  static text { __text }
  static compositionText { __compositionText }
  static compositionRange { __compositionRange }

  foreign static textRegion(x, y, w, h)
  foreign static handleText
  foreign static handleText=(v)

  // PRIVATE, called by game loop
  static update(keyName, state) {
    if (!__keys.containsKey(keyName)) {
      __keys[keyName] = DigitalInput.init()
    }
    __keys[keyName].update(state)
  }

  static commit() {
    __keys.values.each {|key| key.commit() }
  }

  static clearText() {
    __text = ""
  }

  static addText(text) {
    __text = __text + text
    __compositionText = null
  }

  static setComposition(text, start, length) {
    __compositionText = text
    __compositionRange = (start)...(start+length)
  }
}


class Mouse {
  foreign static x
  foreign static y
  foreign static scrollX
  foreign static scrollY
  static position { Vector.new(this.x, this.y) }
  static pos { position }
  static scroll { Vector.new(this.scrollX, this.scrollY) }

  foreign static hidden
  foreign static hidden=(value)
  foreign static relative=(value)
  foreign static relative
  foreign static setCursor(name)
  foreign static cursor=(value)
  foreign static cursor

  static isButtonPressed(key) {
    return Mouse[StringUtils.toLowercase(key)].down
  }

  static init_() {
    __buttons = {}
  }
  static [name] {
    name = StringUtils.toLowercase(name)
    if (!__buttons.containsKey(name)) {
      update(name, false)
    }
    return __buttons[name]
  }
  static allPressed {
    var pressed = {}
    __buttons.keys.where {|key| __buttons[key].down }.each {|key| pressed[key] = __buttons[key] }
    return pressed
  }

  // PRIVATE, called by game loop
  static update(keyName, state) {
    if (!__buttons.containsKey(keyName)) {
      __buttons[keyName] = DigitalInput.init()
    }
    __buttons[keyName].update(state)
  }

  static commit() {
    __buttons.values.each {|button| button.commit() }
  }
}

foreign class SystemGamePad {
  construct open(index) {}

  foreign close()

  foreign attached
  foreign id
  foreign name

  foreign f_getAnalogStick(side)
  foreign getTrigger(side)
  foreign rumble(strength, length)

  getAnalogStick(side) {
    var stick = f_getAnalogStick(side)
    return Vector.new(stick[0], stick[1])
  }
  foreign static f_getGamePadIds()
}

class GamePad {

  construct open(index) {
    _pad = SystemGamePad.open(index)
    _buttons = {}
  }

  close() {
    _pad.close()
  }

  attached { _pad.attached }
  id { _pad.id }
  name { _pad.name }

  [button] {
    var name = StringUtils.toLowercase(button)
    if (!_buttons.containsKey(name)) {
      _buttons[name] = DigitalInput.init()
    }
    return _buttons[name]
  }

  allPressed {
    var pressed = {}
    __buttons.keys.where {|key| __buttons[key].down }.each {|key| pressed[key] = __buttons[key] }
    return pressed
  }

  rumble(strength, length) {
    _pad.rumble(strength, length)
  }

  isButtonPressed(key) {
    return this[key].down
  }

  getTrigger(side) { _pad.getTrigger(side) }
  getAnalogStick(side) { _pad.getAnalogStick(side) }

  // PRIVATE, called by game loop
  update(buttonName, state) {
    var button = this[buttonName]
    button.update(state)
  }
  commit() {
    _buttons.values.each {|button| button.commit() }
  }


  static init_() {
    __pads = {}
    __dummy = GamePad.open(-1)
    SystemGamePad.f_getGamePadIds().each {|id|
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


  static commit() {
    __pads.values.where {|pad| pad.attached }.each {|pad|
      pad.commit()
    }
  }

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
}

class Clipboard {
  foreign static content
  foreign static content=(v)
}

// Module Setup
Input.f_captureVariables()
GamePad.init_()
Keyboard.init_()
Mouse.init_()
