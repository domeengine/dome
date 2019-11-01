class Point {
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

  +(other) {
    if (!(other is Point)) Fiber.abort("Points can only be subtracted from other points.")
    return Point.new(_x + other.x, _y + other.y)
  }
  -(other) {
    if (!(other is Point)) Fiber.abort("Points can only be subtracted from other points.")
    return Point.new(_x - other.x, _y - other.y)
  }

  toString { "{%(_x), %(_y)}" }

}
