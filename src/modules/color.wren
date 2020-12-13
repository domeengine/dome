import "math" for HexToNum, NumToHex, HexDigitToNum, Vector
import "dome" for StringUtils


var ShortColorDigit = Fn.new {|digit|
  digit = HexDigitToNum.call(digit)
  return digit << 4 | digit
  // Digit is interpreted as written twice, e.g. #abc is equal to #aabbcc
}

class Color is Vector {
  static hsv(h, s, v) {
    return hsv(h, s, v, 255)
  }
  static hsv(h, s, v, a) {
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

    var r = (rP + m) * 255
    var g = (gP + m) * 255
    var b = (bP + m) * 255
    return Color.new(r, g, b, a)
  }

  static rgb(r, g, b) {
    return Color.new(r, g, b, 255)
  }
  static rgb(r, g, b, a) {
    return Color.new(r, g, b, a)
  }
  static hex(hex) {
    if (hex is String) {
      var offset = 0
      if (hex[0] == "#") {
        offset = 1
      }

      var r
      var g
      var b
      var a

      if ((hex.bytes.count - offset) == 3 || (hex.bytes.count - offset) == 4) {
        // Short color, e.g. #fff intepreted as #ffffff
        r = ShortColorDigit.call(hex.bytes[offset])
        g = ShortColorDigit.call(hex.bytes[offset + 1])
        b = ShortColorDigit.call(hex.bytes[offset + 2])
        a = (hex.bytes.count - offset) == 4 ? ShortColorDigit.call(hex.bytes[offset + 3]) : 255
      } else {
        r = HexToNum.call(StringUtils.subString(hex, offset + 0, 2))
        g = HexToNum.call(StringUtils.subString(hex, offset + 2, 2))
        b = HexToNum.call(StringUtils.subString(hex, offset + 4, 2))
        a = (hex.bytes.count - offset) == 8 ? HexToNum.call(StringUtils.subString(hex, offset + 6, 2)) : 255
      }
      return Color.new(r, g, b, a)
    } else {
      Fiber.abort("Color only supports hexcodes as strings or numbers")
    }
  }

  construct new() {
    super()
  }
  construct new(r, g, b) {
    super(r, g, b, 255)
  }
  construct new(r, g, b, a) {
    super(r, g, b, a)
  }

  toNum {
    return a << 24 | b << 16 | g << 8 | r
  }

  toString {
    var nums = r << 24 | g << 16 | b << 8 | a
    var hexString = NumToHex.call(nums)
    while (hexString.count < 8) {
      hexString = "0%(hexString)"
    }
    return "Color (#%(hexString))"
  }

  a { w }
  r { x }
  g { y }
  b { z }
  a=(v) { w = v }
  r=(v) { x = v }
  g=(v) { y = v }
  b=(v) { z = v }

  static none { AllColors["none"] }
  static black { AllColors["black"] }
  static darkblue { AllColors["darkblue"] }
  static purple { AllColors["purple"] }
  static darkpurple { AllColors["darkpurple"] }
  static darkgreen { AllColors["darkgreen"] }
  static brown { AllColors["brown"] }
  static darkgray { AllColors["darkgray"] }
  static lightgray { AllColors["lightgray"] }
  static white { AllColors["white"] }
  static red { AllColors["red"] }
  static orange { AllColors["orange"] }
  static yellow { AllColors["yellow"] }
  static green { AllColors["green"] }
  static blue { AllColors["blue"] }
  static indigo { AllColors["indigo"] }
  static pink { AllColors["pink"] }
  static peach { AllColors["peach"] }
}

var AllColors = {
  "black": Color.rgb(0, 0, 0, 255),
  "darkblue": Color.rgb(29, 43, 83),
  "purple": Color.rgb(141, 60, 255),
  "darkpurple": Color.rgb(126, 37, 83),
  "darkgreen": Color.rgb(0, 135, 81),
  "brown": Color.rgb(171, 82, 54),
  "darkgray": Color.rgb(95, 87, 79),
  "lightgray": Color.rgb(194, 195, 199),
  "white": Color.rgb(255, 255, 255),
  "red": Color.rgb(255, 0, 77),
  "orange": Color.rgb(255, 163, 0),
  "yellow": Color.rgb(255, 236, 39),
  "green": Color.rgb(0, 228, 54),
  "blue": Color.rgb(41, 173, 255),
  "indigo": Color.rgb(131, 118, 156),
  "pink": Color.rgb(255, 119, 168),
  "peach": Color.rgb(255, 204, 170),
  "none": Color.rgb(0, 0, 0, 0)
}
