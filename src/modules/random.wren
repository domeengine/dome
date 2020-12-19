import "platform" for Platform
var BIT_NOISE1 = 0xB5297A4D
var BIT_NOISE2 = 0x68E31DA4
var BIT_NOISE3 = 0x1B56C4E9
var CAP = 0xFFFFFFFF

class Squirrel3 {
  static noise(x) { noise(x, 0) }
  foreign static noise(x, seed)
  /*
  static noise(x, seed) {
    var mangled = x
    mangled = mangled * BIT_NOISE1
    mangled = mangled + seed
    mangled = mangled ^ (mangled >> 8)
    mangled = mangled + BIT_NOISE2
    mangled = mangled ^ (mangled << 8)
    mangled = mangled * BIT_NOISE3
    mangled = mangled ^ (mangled >> 8)
    return mangled & CAP
  }
  */

  static new() { Squirrel3.new(Platform.time) }
  construct new(seed) {}
  /*
  construct new() {
    _state = 0
    _seed = Platform.time
  }

  construct new(seed) {
    _state = 0
    _seed = seed
  }


  float() {
    var result = Squirrel3.noise(_state, _seed)
    _state = _state + 1
    return result / CAP
  }
  */
  foreign float()
  float(end) { float() * end }
  float(start, end) { start + float(end - start) }
  int(end) { float(end).floor }
  int(start, end) { float(start, end).floor }

  sample(list) { list[int(list.count)] }

  sample(list, count) {
    if (count > list.count) {
      Fiber.abort("Cannot sample more items than the list contains")
    }
    var newList = shuffle(list.toList)
    var end = list.count - 1
    for (i in end..count) {
      newList.removeAt(i)
    }
    return newList
  }

  shuffle(list) {
    var n = list.count
    var j
    for (i in 0...n) {
      j = int(i + 1)
      list.swap(j, i)
    }
    return list
  }
}

var Random = Squirrel3
