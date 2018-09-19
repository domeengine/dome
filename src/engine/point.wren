foreign class Point {
  construct new() {}
  construct new(x, y) {}
  foreign x
  foreign x=(v)
  foreign y
  foreign y=(v)

  +(other) {
    if (!(other is Point)) Fiber.abort("Points can only be subtracted from other points.")
    return Point.new(x + other.x, y + other.y)
  }
  -(other) {
    if (!(other is Point)) Fiber.abort("Points can only be subtracted from other points.")
    return Point.new(x - other.x, y - other.y)
  }

  toString { "{%(x), %(y)}" }
}
