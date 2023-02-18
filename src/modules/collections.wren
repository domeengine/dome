// This implements some common data types
var WarningEmitted = false
var HashableTypes = [ Num, String, Range, Bool, Class ]
var DEFAULT_MAX_COMPARATOR = Fn.new {|a, b| a[0] > b[0] }
var DEFAULT_MIN_COMPARATOR = Fn.new {|a, b| a[0] < b[0] }
class Hashable {
  hash() { this.toString }
}

class Stack {
  construct new() {
    _list = []
  }
  isEmpty { _list.isEmpty }
  count { _list.count }

  push(v) {
    _list.add(v)
  }
  peek() { _list[-1] }

  pop() {
    return _list.removeAt(-1)
  }

  add(v) { push(v) }
  remove() { pop() }
  get() { peek() }

  list() { _list[0..-1] }
  iterate(iter) { _list.iterate(iter) }
  iteratorValue(iter) { _list.iteratorValue(iter) }
}

class Set {
  construct new() {
    _map = {}
  }

  isEmpty { _map.isEmpty }
  count { _map.count }

  has(value) {
    var hash = value
    if (value is Hashable) {
      hash = value.hash()
    }

    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      Fiber.abort("Set: %(value) could not be hashed.")
    }

    return _map.containsKey(hash)
  }

  remove(value) {
    var hash = value
    if (value is Hashable) {
      hash = value.hash()
    }
    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      Fiber.abort("Set: %(value) could not be hashed.")
    }
    return _map.remove(hash)
  }

  get(value) {
    var hash = value
    if (value is Hashable) {
      hash = value.hash()
    }
    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      Fiber.abort("Set: %(value) could not be hashed.")
    }
    return _map[hash]
  }

  add(value) {
    var hash = value
    if (value is Hashable) {
      hash = value.hash()
    }
    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      Fiber.abort("Set: %(value) could not be hashed.")
    }
    _map[hash] = value
  }

  list() { _list[0..-1] }
  iterate(iter) { _map.values.iterate(iter) }
  iteratorValue(iter) { _map.values.iteratorValue(iter) }
}

// A FIFO queue
class Queue {
  construct new() {
    _list = []
  }
  add(item) { _list.add(item) }
  remove() { _list.removeAt(0) }
  get() { _list[0] }

  enqueue(item) { _list.add(item) }
  dequeue() { _list.removeAt(0) }
  peek() { _list[0] }

  // Returns a copy of underlying list
  list() { _list[0..-1] }

  isEmpty { _list.isEmpty }
  count { _list.count }

  iterate(iter) { _list.iterate(iter) }
  iteratorValue(iter) { _list.iteratorValue(iter) }
}

class PriorityQueue {
  static min() {
    return new(DEFAULT_MIN_COMPARATOR)
  }
  static max() {
    return new(DEFAULT_MAX_COMPARATOR)
  }

  construct new(comparator) {
    _heap = Heap.new(comparator)
    _comparator = comparator
  }

  add(item) { put(item, item) }
  add(item, priority) {
    _heap.insert([priority, item])
  }

  get() { peek() }
  peek() {
    return _heap.peek()[1]
  }

  remove() {
    return _heap.remove()[1]
  }

  isEmpty { _heap.isEmpty }
  count { _heap.count }
  list { _heap.list.sort(_comparator) }
}

class Heap {
  construct new() {
    init(Fn.new {|a, b| a < b })
  }
  construct new(comparator) {
    init(comparator)
  }

  init(comparator) {
    _comparator = comparator
    _list = []
    _size = 0
  }

  isEmpty { _list.isEmpty }
  count { _size }
  list { _list[0..-1].sort(_comparator) }

  add(element) {
    _list.insert(0, element)
    percolateDown(0)
  }

  get() { peek() }
  peek() {
    if (_list.count == 0) {
      return null
    }
    return _list[0]
  }

  remove() {
    if (_list.count == 0) {
      return null
    }
    if (_list.count == 1) {
      return _list.removeAt(0)
    }
    var top = _list[0]
    var last = _list.count - 1
    swap(0, last)
    _list.removeAt(last)
    percolateUp(0)
    percolateDown(0)
    // percolate root down
    return top
  }

  swap(i1, i2) {
    var temp = _list[i1]
    _list[i1] = _list[i2]
    _list[i2] = temp
  }

  compare(a, b) {
    return _comparator.call(a, b)
  }

  percolateUp(pos) {
    while (pos > 1) {
      var parent = (pos/2).floor
      if (compare(_list[pos], _list[parent]) >= 0) {
        break
      }
      swap(parent, pos)
      pos = parent
    }
  }


  percolateDown(pos) {
    var last = _list.count - 1
    while (true) {
      var min = pos
      var child = 2 * pos
      for (c in child .. child + 1) {
        if (c <= last && compare(_list[c], _list[min])) {
          min = c
        }
      }

      if (min == pos) {
        break
      }

      swap(pos, min)
      pos = min
    }
  }

}
