import "random" for Random
import "dome" for Process

class Game {
  static init() {
    for (n in 0...100) {
      var start = System.clock
      var RNG = Random.new()
      var n = 0

      for (i in 0...1000000) {
        n = n + RNG.float()
      }
      n = n / 1000000

      var end = System.clock
      System.print("Time: %(end - start) seconds")
    }
    Process.exit()
  }
  static update() {}
  static draw(d) {}


}
