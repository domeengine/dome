[< Back](.)

# collections

The `collections` module has a few useful "abstract data types" which can serve a variety of purposes.
They enforce additional semantics on Wren's `List` and `Map` datatypes to achieve particular properties.

It contains the following classes:

- [Hashable](#hashable)
- [HashMap](#hashmap)
- [Queue](#queue)
- [PriorityQueue](#priorityqueue)
- [Set](#set)
- [Stack](#stack)

## Hashable
Certain collections in this module use hashing to store values efficiently.
But this means that the keys used are actually hashable types.

By default, this means you can only store: 
* `null`
* Num
* String
* Boolean
* Range
* Class

You can store more complex types if the object supports DOME's `Hashable` interface.

### `hash(): Num | String | Boolean | Range | Class`
This method hashes the object in some unique fashion.
It can return any hashable type for use in `Set`.

## HashMap
Implements [Sequence](https://wren.io/modules/core/sequence.html)

This is like Wren's built-in `Map`, but it works with keys that support DOME's `Hashable`
interface, such as [`Vector`](math#vector).

### Constructor
#### `new(): Queue`
Creates a new queue.
### Instance Fields

#### `count: Num`
Returns the total number of values stored in the HashMap.

#### `isEmpty: Num`
Returns true if there are no values stored in the HashMap.

#### `keys: Sequence`
Returns a sequence of all the keys stored in the HashMap.

#### `values: Sequence`
Returns a sequence of all the values stored in the HashMap.

#### `entries: Sequence<MapEntry>`
Returns a sequence of MapEntry, which holds key-value pairs contained
in the HashMap.

### Instance Methods
#### `clear()`
Clears all values from the HashMap.

#### `containsKey(key: Hashable): Boolean`
Returns true if there is a value stored using the provided key.

#### `has(key: Hashable): Boolean`
Same as `containsKey` but shorter for convenience

#### `[key: Hashable]: Any`
Retrieves the value matched to `key` in the HashMap. 
If there is no corresponding value, `null` is returned.

#### `[key: Hashable]=(value)`
Stores `value` matched to `key` in the HashMap. 
If there is already a corresponding value for `key`, it is overwritten.

## Queue
Implements [Sequence](https://wren.io/modules/core/sequence.html)

Like a queue in real life, the `queue` in DOME enforces a "First-in-First-out" (FIFO) ordering to elements added to it.
You cannot insert an element in the middle of the queue, only at the end. In addition, you can only view and retrieve 
the element at the front of the queue.

`Queue` implements Wren's iterator protocol so you can traverse it using a for-loop.

### Constructor

#### `new(): Queue`
Creates a new queue.

### Instance Fields
#### `isEmpty: Boolean`
Returns true if the queue is empty (aka `count == 0`).

#### `count: Num`
The number of elements in the queue.

### Instance Methods
#### `add(v: any)`
Place `v` at the end of the queue.
#### `peek(): any`
Returns the element at the front of the queue.
#### `remove(): any`
Removes the element at the front of the queue, and returns it.

## PriorityQueue
Implements [Sequence](https://wren.io/modules/core/sequence.html)

A priority queue is a collection where elements can be added to the queue in any order (with a priority supplied), 
but elements can only be retrieved in priority order.

### Constructor

#### `new(): PriorityQueue`
#### `min(): PriorityQueue`
Creates a new queue where the lowest priority is served first.

#### `max(): PriorityQueue`
Creates a new queue where the highest priority is served first.

#### `new(comparator: Fn<a: Any, b: Any>: Boolean): PriorityQueue`
Creates a new queue which uses the given comparator to sort its elements.

### Instance Fields
#### `isEmpty: Boolean`
Returns true if the queue is empty (aka `count == 0`).

#### `count: Num`
The number of elements in the queue.

#### `toList: List<Any>`
Returns a List with all the contained elements in the current order.

### Instance Methods
#### `add(v: any)`
Place `v` in the queue. The value `v` itself is used for priority comparison.

#### `add(v: any, priority: any)`
Place `v` in the queue. The value `priority` is used for priority comparison.

#### `peek(): any`
Returns the element at the front of the queue.
#### `remove(): any`
Removes the element at the front of the queue, and returns it.


## Set

Implements [Sequence](https://wren.io/modules/core/sequence.html)

A `Set` is an unordered collection which can only contain an element once. 
The items placed in the set must be [`Hashable`](#hashable)
`Set` implements Wren's iterator protocol so you can traverse it using a for-loop.

### Constructor

#### `new(): Set`
Creates a new set.

### Instance Fields
#### `isEmpty: Boolean`
Returns true if the set is empty (aka `count == 0`).

#### `count: Num`
The number of elements in the set.

### Instance Methods
#### `add(v: any)`
Place `v` in the set. The value `v` itself is used for priority comparison.
#### `has(v: any): Boolean`
Returns `true` if the set contains `v`.

#### `get(v: any): any`
If the set contains `v`, it is returned.

#### `remove(v: any): any`
Removes the element `v` from the set, and returns it.

## Stack

Like a stack in real life, the `stack` in DOME enforces a "Lass-in-first-out" (LIFO) ordering to elements added to it.
You cannot insert an element in the middle of the stack, only at the top. In addition, you can only view and retrieve 
the element at the top of the stack.

`Stack` implements Wren's iterator protocol so you can traverse it using a for-loop.

### Constructor

#### `new(): Stack`
Creates a new stack.

### Instance Fields
#### `isEmpty: Boolean`
Returns true if the stack is empty (aka `count == 0`).

#### `count: Num`
The number of elements in the stack.

### Instance Methods
#### `add(v: any)`
Place `v` at the end of the stack.
#### `peek(): any`
Returns the element at the front of the stack.
#### `remove(): any`
Removes the element at the front of the stack, and returns it.
#### `list(): List<Any>`
Returns a List with all the contained elements in the current order.
