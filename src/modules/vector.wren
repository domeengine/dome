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
  static fromPair(z) {
    return Elegant.unpair(z)
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
    return x.abs + y.abs + z.abs + w.abs
  }

  length {
    return (x.pow(2) + y.pow(2) + z.pow(2) + w.pow(2)).sqrt
  }

  unit {
    if (length == 0) {
      return type.new()
    }
    return type.new(x / length, y / length, z / length, w / length)
  }

  perp {
    return type.new(-y, x)
  }

  dot(other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return x * other.x + y * other.y + z * other.z + w * other.w
  }

  cross(other) {
    return type.new(
      y * other.z - z * other.y,
      z * other.x - x * other.z,
      x * other.y - y * other.x
    )
  }

  + (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return type.new(x + other.x, y + other.y, z + other.z, w + other.w)
  }
  - (other) {
    if (!(other is Vector)) Fiber.abort("Vectors can only be subtracted from other points.")
    return type.new(x - other.x, y - other.y, z - other.z, w - other.w)
  }

  / (other) {
    if (other is Num) {
      // Scale by other
      return type.new(x / other, y / other, z / other, w / other)
    } else {
      Fiber.abort("Vectors can only be divided by scalar values.")
    }
  }

  * (other) {
    if (other is Num) {
      // Scale by other
      return type.new(x * other, y * other, z * other, w * other)
    } else {
      Fiber.abort("Vectors can only be multiplied by scalar values.")
    }
  }

  - {
    return type.new(-x, -y, -z, -w)
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

  pair { Elegant.pair(x, y) }
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

// Converts two integers into a single integer, good for hashing
// Supports negative numbers too
class Elegant {
  static pair(vec) {
    if (vec is Vector) {
      return pair(vec.x.floor, vec.y.floor)
    } else {
      Fiber.abort("Elegant.pair: %(vec) was not a vector.")
    }
  }

  foreign static pair(x, y)
  foreign static unpair_i(z)

  static unpairAsList(z) {
    return unpair_i(z)
  }
  static unpair(z) {
    var res = unpair_i(z)
    return Vector.new(res[0], res[1])
  }
}

var Point = Vector
var Vec = Vector
