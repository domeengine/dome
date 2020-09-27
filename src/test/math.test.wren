import "math" for Math
import "./unit" for Assert

class MathTests {
  static name { "modules/math.wren" }

  static thatSinWorks {[
      "that `sin` function works",
      Fiber.new {
          Assert.equal(Math.sin(2), 2.sin)
      }
  ]}

  static thatCosWorks {[
      "that `cos` function works",
      Fiber.new {
          Assert.equal(Math.cos(2), 2.cos)
      }
  ]}

  static thatTanWorks {[
      "that `tan` function works",
      Fiber.new {
          Assert.equal(Math.tan(2), 2.tan)
      }
  ]}

  static all {[
    thatSinWorks,
    thatCosWorks,
    thatTanWorks
  ]}
}
