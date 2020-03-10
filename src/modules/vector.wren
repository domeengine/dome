class Vector {
  construct new() {
    init_(0, 0, 0, 0)
  }
  construct new(x, y) {
    init_(x, y, 0, 0)
  }
  construct new(x, y, z) {
    init_(x, y, z, 0)
  }
  construct new(x, y, z, w) {
    init_(x, y, z, w)
  }

  init_(x, y, z, w) {
    _x = x
    _y = y
    _z = z
    _w = w
  }

  x { _x }
  y { _y }
  z { _z }
  w { _w }
  x=(v) { _x = v }
  y=(v) { _y = v }
  z=(v) { _z = v }
  w=(v) { _w = v }

  manhattan {
    return _x.abs + _y.abs + _z.abs + _w.abs
  }

  length {
    return (_x.pow(2) + _y.pow(2) + _z.pow(2) + _w.pow(2)).sqrt
  }

  unit {
    if (length == 0) {
      return Vector.new()
    }
    return Vector.new(_x / length, _y / length, _z / length, _w / length)
  }

  perp {
    return Vector.new(-_y, _x)
  }

  dot(other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return _x * other.x + _y * other.y + _z * other.z + _w * other.w
  }

  + (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(_x + other.x, _y + other.y, _z + other.z, _w + other.w)
  }
  - (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(_x - other.x, _y - other.y, _z - other.z, _w - other.w)
  }

  / (other) {
    if (other is Num) {
      // Scale by other
      return Vector.new(_x / other, _y / other, _z / other, _w / other)
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

  ==(other) {
    if (other is Vector) {
      return other.x == _x && other.y == _y && other.z == _z && other.w == _w
    } else {
      return false
    }
  }
  !=(other) {
    if (other is Vector) {
      return other.x != _x || other.y != _y || other.z != _z || other.w != _w
    } else {
      return true
    }
  }
  toString {
    if (_z == 0 && _w == 0) {
      return "(%(_x), %(_y))"
    } else if (_w == 0) {
      return "(%(_x), %(_y), %(_z))"
    } else {
      return "(%(_x), %(_y), %(_z), %(_w))"
    }
  }
}

var Point = Vector
var Vec = Vector
