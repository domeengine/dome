import "random" for Random
import "dome" for Process

class Main {
  construct new() {}

  init() {
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

  update() {}
  draw(d) {}
}

var Game = Main.new()

