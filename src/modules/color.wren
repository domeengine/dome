import "math" for HexToNum, NumToHex, HexDigitToNum
import "dome" for StringUtils


var ShortColorDigit = Fn.new {|digit|
  digit = HexDigitToNum.call(digit)
  return digit << 4 | digit
  // Digit is interpreted as written twice, e.g. #abc is equal to #aabbcc
}

class Color {
  construct hex(hex) {
    if (hex is String) {
      var offset = 0
      if (hex[0] == "#") {
        offset = 1
      }

      if ((hex.bytes.count - offset) == 3 || (hex.bytes.count - offset) == 4) {
        // Short color, e.g. #fff intepreted as #ffffff
        _r = ShortColorDigit.call(hex.bytes[offset])
        _g = ShortColorDigit.call(hex.bytes[offset + 1])
        _b = ShortColorDigit.call(hex.bytes[offset + 2])
        _a = (hex.bytes.count - offset) == 4 ? ShortColorDigit.call(hex.bytes[offset + 3]) : 255
      } else {
        _r = HexToNum.call(StringUtils.subString(hex, offset + 0, 2))
        _g = HexToNum.call(StringUtils.subString(hex, offset + 2, 2))
        _b = HexToNum.call(StringUtils.subString(hex, offset + 4, 2))
        _a = (hex.bytes.count - offset) == 8 ? HexToNum.call(StringUtils.subString(hex, offset + 6, 2)) : 255
      }
    } else {
      Fiber.abort("Color only supports hexcodes as strings or numbers")
    }
  }

  construct new(r, g, b) {
    System.print("Color.new(_,_,_) is deprecated. Please use Color.rgb(_,_,_) instead.")
    setrgb(r, g, b, 255)
  }
  construct new(r, g, b, a) {
    System.print("Color.new(_,_,_,_) is deprecated. Please use Color.rgb(_,_,_,_) instead.")
    setrgb(r, g, b, a)
  }

  construct rgb(r, g, b) {
    setrgb(r, g, b, 255)
  }
  construct rgb(r, g, b, a) {
    setrgb(r, g, b, a)
  }
  construct hsv(h, s, v, a) {
    setHSV(h, s, v)
    _a = a
  }
  construct hsv(h, s, v) {
    setHSV(h, s, v)
    _a = 255
  }

  setrgb(r, g, b, a) {
    _r = r
    _g = g
    _b = b
    _a = a
  }

  setHSV(h, s, v) {
    h = h % 360
    if (0 < s && s > 1) {
      Fiber.abort("Color component S is out of bounds")
    }
    if (0 < v && v > 1) {
      Fiber.abort("Color component V is out of bounds")
    }

    var c = v * s
    var x = c * (1 - (((h / 60) % 2) - 1).abs)
    var m = v - c

    var rP
    var gP
    var bP
    if (0 <= h && h < 60) {
      rP = c
      gP = x
      bP = 0
    } else if (60 <= h && h< 120) {
      rP = x
      gP = c
      bP = 0
    } else if (120 <= h && h < 180) {
      rP = 0
      gP = c
      bP = x
    } else if (180 <= h && h< 240) {
      rP = 0
      gP = x
      bP = c
    } else if (240 <= h && h < 300) {
      rP = x
      gP = 0
      bP = c
    } else if (300 <= h && h < 360) {
      rP = c
      gP = 0
      bP = x
    } else {
      Fiber.abort("Invalid H value")
    }

    _r = (rP + m) * 255
    _g = (gP + m) * 255
    _b = (bP + m) * 255
    _a = 255
  }

  toNum {
    return a << 24 | b << 16 | g << 8 | r
  }

  toString {
    var nums = _r << 24 | _g << 16 | _b << 8 | _a
    var hexString = NumToHex.call(nums)
    while (hexString.count < 8) {
      hexString = "0%(hexString)"
    }
    return "Color (#%(hexString))"
  }

  a { _a }
  r { _r }
  g { _g }
  b { _b }

  static none { None }
  static black { Black }
  static darkblue { DarkBlue }
  static purple { Purple }
  static darkpurple { DarkPurple }
  static darkgreen { DarkGreen }
  static brown { Brown }
  static darkgray { DarkGray }
  static lightgray { LightGray }
  static white { White }
  static red { Red }
  static orange { Orange }
  static yellow { Yellow }
  static green { Green }
  static blue { Blue }
  static indigo { Indigo }
  static pink { Pink }
  static peach { Peach }
}

var Black = Color.rgb(0, 0, 0)
var DarkBlue = Color.rgb(29, 43, 83)
var Purple = Color.rgb(141, 60, 255)
var DarkPurple = Color.rgb(126, 37, 83)
var DarkGreen = Color.rgb(0, 135, 81)
var Brown = Color.rgb(171, 82, 54)
var DarkGray = Color.rgb(95, 87, 79)
var LightGray = Color.rgb(194, 195, 199)
var White = Color.rgb(255, 255, 255)
var Red = Color.rgb(255, 0, 77)
var Orange = Color.rgb(255, 163, 0)
var Yellow = Color.rgb(255, 236, 39)
var Green = Color.rgb(0, 228, 54)
var Blue = Color.rgb(41, 173, 255)
var Indigo = Color.rgb(131, 118, 156)
var Pink = Color.rgb(255, 119, 168)
var Peach = Color.rgb(255, 204, 170)
var None = Color.rgb(0, 0, 0, 0)
