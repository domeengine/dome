// Import vector for convenience
class Math {
  static assertNum(n) {
    if (!n is Num) {
      Fiber.abort("%(n) is not of type Num")
    }
  }

  static sin(n) {
    assertNum(n)
    return n.sin
  }
  static cos(n) {
    assertNum(n)
    return n.cos
  }
  static tan(n) {
    assertNum(n)
    return n.tan
  }
  static asin(n) {
    assertNum(n)
    return n.asin
  }
  static acos(n) {
    assertNum(n)
    return n.acos
  }
  static atan(n) {
    assertNum(n)
    return n.atan
  }
  static atan(n, x) {
    assertNum(n)
    assertNum(x)
    return n.atan(x)
  }
  static log(n) {
    assertNum(n)
    return n.log
  }
  static ceil(n) {
    assertNum(n)
    return n.ceil
  }
  static floor(n) {
    assertNum(n)
    return n.floor
  }
  static round(n) {
    assertNum(n)
    return n.round
  }

  static abs(n) {
    assertNum(n)
    return n.abs
  }

  static max(a, b) {
    assertNum(a)
    assertNum(b)
    if (a > b) {
      return a
    } else {
      return b
    }
  }

  static min(a, b) {
    assertNum(a)
    assertNum(b)
    if (a < b) {
      return a
    } else {
      return b
    }
  }

  static sign(a) {
    return a.sign
  }

  static mid(a, b, c) {
    assertNum(a)
    assertNum(b)
    assertNum(c)

    var swap
    if (a > c) {
      swap = a
      a = c
      c = swap
    }
    if (a > b) {
      swap = a
      a = b
      b = swap
    }
    if (b < c) {
      return b
    } else {
      return c
    }
  }

  static lerp(low, value, high) {
    assertNum(low)
    assertNum(value)
    assertNum(high)

    var v = mid(0, value, 1)
    return (1 - v) * low + v * high
  }
}

var ToHex = Fn.new {|dec|
  if (dec < 10) {
    return String.fromByte(dec + 48)
  } else if (dec < 16) {
    return String.fromByte((dec - 10) + 65)
  }
}
var NumToHex = Fn.new {|num|
  if (num == 0) {
    return "0"
  }
  var value = num
  var strings = []
  while (value > 0) {
    var remainder = value % 16
    strings.insert(0, ToHex.call(remainder))
    value = (value / 16).floor
  }
  return strings.join("")
}

var HexToNum = Fn.new {|hex|
  var first = hex[0]
  var second = hex[1]
  if (48 <= first && first <= 57) {
    first = first - 48
  } else if (65 <= first && first <= 70) {
    first = 10 + first - 65
  } else if (97 <= first && first <= 102) {
    first = 10 + first - 97
  } else {
    Fiber.abort("Invalid hex")
  }
  if (48 <= second && second <= 57) {
    second = second - 48
  } else if (65 <= second && second <= 70) {
    second = 10 + second - 65
  } else if (97 <= second && second <= 102) {
    second = 10 + second - 97
  } else {
    Fiber.abort("Invalid hex")
  }
  return first << 4 | second
}

// Expose a shorthand name for the class
var M = Math

import "vector" for Vector
var Point = Vector
var Vec = Vector
