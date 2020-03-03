class Vector {
  construct new() {
    _x = 0
    _y = 0
  }
  construct new(x, y) {
    _x = x
    _y = y
  }
  x { _x }
  y { _y }
  x=(v) { _x = v }
  y=(v) { _y = v }

  manhattan {
    return _x.abs + _y.abs
  }

  length {
    return (_x.pow(2) + _y.pow(2)).sqrt
  }

  unit {
    return Vector.new(_x / length, _y / length)
  }

  perp {
    return Vector.new(-_y, _x)
  }

  dot(other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return _x * other.x + _y * other.y
  }

  + (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(_x + other.x, _y + other.y)
  }
  - (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(_x - other.x, _y - other.y)
  }

  / (other) {
    if (other is Num) {
      // Scale by other
      return Vector.new(_x / other, _y / other)
    } else {
      Fiber.abort("Vectors can only be divided by scalar values.")
    }
  }

  * (other) {
    if (other is Num) {
      // Scale by other
      return Vector.new(_x * other, _y * other)
    } else {
      Fiber.abort("Vectors can only be multiplied by scalar values.")
    }
  }

  - {
    return Vector.new(-_x, -_y)
  }

  toString { "(%(_x), %(_y))" }

}

var Point = Vector
var Vec = Vector
