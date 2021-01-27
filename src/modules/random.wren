import "platform" for Platform

class Squirrel3 {
  static noise(x) { noise(x, 0) }
  foreign static noise(x, seed)

  static new() { Squirrel3.new(Platform.time) }
  construct new(seed) {}

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
