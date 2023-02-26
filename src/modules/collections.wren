// This implements some common data types
var HashableTypes = [ Num, String, Range, Bool, Class ]
var DEFAULT_MAX_COMPARATOR = Fn.new {|a, b| a[0][0] > b[0][0] || (a[0][0] == b[0][0] && a[0][1] < b[0][1]) }
var DEFAULT_MIN_COMPARATOR = Fn.new {|a, b| a[0][0] < b[0][0] || (a[0][0] == b[0][0] && a[0][1] < b[0][1]) }
class Hashable {
  hash() { this.toString }
}

var HashValue =  Fn.new {|v|
  return v.hash()
}

class Stack is Sequence {
  construct new() {
    _list = []
  }
  isEmpty { _list.isEmpty }
  count { _list.count }

  push(v) {
    _list.add(v)
  }
  peek() {
    if (_list.isEmpty) {
      return null
    }
    return _list[-1]
  }
  pop() {
    if (_list.isEmpty) {
      return null
    }
    return _list.removeAt(-1)
  }
  clear() { _list.clear() }

  add(v) { push(v) }
  remove() { pop() }
  get() { peek() }

  iterate(iter) { _list.iterate(iter) }
  iteratorValue(iter) { _list.iteratorValue(iter) }
}

class HashMap is Sequence {
  construct new() {
    _map = {}
  }

  clear() { _map.clear() }
  remove(key) {
    _map.remove(hashValue(key))
  }
  count { _map.count }
  containsKey(key) { _map.containsKey(hashValue(key)) }
  has(key) { containsKey(key) }
  keys { _map.values.map{|entry| entry.key } }
  values { _map.values.map{|entry| entry.value } }
  entries { _map.values }

  [key]=(v) {
    var hash = hashValue(key)
    _map[hash] = MapEntry.new(key, v)
  }

  [key] {
    var hash = hashValue(key)
    return _map[hash].value
  }

  hashValue(v) {
    var hash = v
    if (hash == null) {
      return null
    }
    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      var fiber = Fiber.new(HashValue)
      hash = fiber.try(v)
      if (fiber.error != null || (hash != null && !HashableTypes.any {|type| hash is type })) {
        Fiber.abort("Set: %(v) could not be hashed. %(fiber.error)")
      }
    }
    return hash
  }

  iterate(iterator) { entries.iterate(iterator) }
  iteratorValue(iterator) { entries.iteratorValue(iterator) }
}

class Set is Sequence {
  construct new() {
    _map = {}
  }

  isEmpty { _map.isEmpty }
  count { _map.count }

  has(value) {
   var hash = hashValue(value)
   return _map.containsKey(hash)
  }

  remove(value) {
    var hash = hashValue(value)
    return _map.remove(hash)
  }

  get(value) {
    var hash = hashValue(value)
    return _map[hash]
  }

  add(value) {
    var hash = hashValue(value)
    _map[hash] = value
  }
  clear() { _map.clear() }

  hashValue(v) {
    var hash = v
    if (hash != null && !HashableTypes.any {|type| hash is type }) {
      var fiber = Fiber.new(HashValue)
      hash = fiber.try(v)
      if (fiber.error != null || (hash != null && !HashableTypes.any {|type| hash is type })) {
        Fiber.abort("Set: %(v) could not be hashed. %(fiber.error)")
      }
    }
    return hash

  }

  iterate(iter) { _map.values.iterate(iter) }
  iteratorValue(iter) { _map.values.iteratorValue(iter) }
}

// A FIFO queue
class Queue is Sequence {
  construct new() {
    _list = []
  }
  add(item) { _list.add(item) }
  remove() {
    if (_list.isEmpty) {
      return null
    }
    return _list.removeAt(0)
  }
  clear() { _list.clear() }
  get() {
    if (_list.isEmpty) {
      return null
    }
    return _list[0]
  }

  enqueue(item) { _list.add(item) }
  dequeue() {
    if (_list.isEmpty) {
      return null
    }
    return _list.removeAt(0)
  }
  peek() {
    if (_list.isEmpty) {
      return null
    }
    return _list[0]
  }

  count { _list.count }

  iterate(iter) { _list.iterate(iter) }
  iteratorValue(iter) { _list.iteratorValue(iter) }
}

class PriorityQueue is Sequence {
  static min() {
    return new(DEFAULT_MIN_COMPARATOR)
  }
  static max() {
    return new(DEFAULT_MAX_COMPARATOR)
  }
  static new() {
    return new(DEFAULT_MIN_COMPARATOR)
  }

  construct new(comparator) {
    _id = 0
    _heap = Heap.new(comparator)
    _comparator = comparator
    _seq = null
  }

  add(item) { add(item, item) }
  add(item, priority) {
    _id = _id + 1
    _heap.add([[priority, _id], item])
    _seq = null
  }

  get() { peek() }
  peek() {
    return _heap.peek()[1]
  }
  peekPriority() {
    return _heap.peek()[0][0]
  }

  remove() {
    _seq = null
    return _heap.remove()[1]
  }
  clear() {
    _seq = null
    _heap.clear()
  }

  isEmpty { _heap.isEmpty }
  count { _heap.count }
  toTupleList { _heap.toList.map{|tuple| [tuple[0][0], tuple[1]]}.toList }
  sequence {
    if (!_seq) {
      _seq = _heap.toList.map{|tuple| tuple[1] }
    }
    return _seq
  }

  iterate(iter) { sequence.iterate(iter) }
  iteratorValue(iter) { sequence.iteratorValue(iter) }
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

  toList { _list[0..-1].sort(_comparator) }

  add(element) {
    _list.insert(0, element)
    _size = _size + 1
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
    _size = _size - 1
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
  clear() {
    _list.clear()
    _size = 0
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
