import "vector" for Vector
import "unit" for Assert

class VectorTests {
  static name { "modules/vector.wren" }

  static thatEmptyNewVectorWorks {[
      "that `Vector.new()` function works",
      Fiber.new {
          var vector = Vector.new()
          Assert.equal(vector.x, 0)
          Assert.equal(vector.y, 0)
          Assert.equal(vector.z, 0)
          Assert.equal(vector.w, 0)
      }
  ]}

  static all {[
    thatEmptyNewVectorWorks
  ]}
}
