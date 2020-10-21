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
    _values = [x, y, z, w]
  }

  x { _values[0] }
  y { _values[1] }
  z { _values[2] }
  w { _values[3] }
  x=(v) { _values[0] = v }
  y=(v) { _values[1] = v }
  z=(v) { _values[2] = v }
  w=(v) { _values[3] = v }

  manhattan {
    return x.abs + y.abs + z.abs + w.abs
  }

  length {
    return (x.pow(2) + y.pow(2) + z.pow(2) + w.pow(2)).sqrt
  }

  unit {
    if (length == 0) {
      return Vector.new()
    }
    return Vector.new(x / length, y / length, z / length, w / length)
  }

  perp {
    return Vector.new(-y, x)
  }

  dot(other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return x * other.x + y * other.y + z * other.z + w * other.w
  }

  cross(other) {
    return Vector.new(
      y * other.z - z * other.y,
      z * other.x - x * other.z,
      x * other.y - y * other.x
    )
  }

  + (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(x + other.x, y + other.y, z + other.z, w + other.w)
  }
  - (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return Vector.new(x - other.x, y - other.y, z - other.z, w - other.w)
  }

  / (other) {
    if (other is Num) {
      // Scale by other
      return Vector.new(x / other, y / other, z / other, w / other)
    } else {
      Fiber.abort("Vectors can only be divided by scalar values.")
    }
  }

  * (other) {
    if (other is Num) {
      // Scale by other
      return Vector.new(x * other, y * other, z * other, w * other)
    } else {
      Fiber.abort("Vectors can only be multiplied by scalar values.")
    }
  }

  - {
    return Vector.new(-x, -y, -z, -w)
  }

  ==(other) {
    if (other is Vector) {
      return other.x == x && other.y == y && other.z == z && other.w == w
    } else {
      return false
    }
  }
  !=(other) {
    if (other is Vector) {
      return other.x != x || other.y != y || other.z != z || other.w != w
    } else {
      return true
    }
  }
  toString {
    if (z == 0 && w == 0) {
      return "(%(x), %(y))"
    } else if (w == 0) {
      return "(%(x), %(y), %(z))"
    } else {
      return "(%(x), %(y), %(z), %(w))"
    }
  }
}

var Point = Vector
var Vec = Vector
