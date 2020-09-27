import "unit" for Runner
import "./math.test" for MathTests
import "./vector.test" for VectorTests

class Game {
    static init() {
        // Add your tests here
        Runner.run(MathTests)
        Runner.run(VectorTests)

        Runner.end()
    }
}
