
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

var M = Math
