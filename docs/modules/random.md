[< Back](.)

random
=============

The `random` module provides utilities for generating pseudo-random numbers, for a variety of applications. Please note, this module should not be used for applications which require a cryptographically secure source of random numbers.

DOME's provides two pseudo-random number generators - the "Squirrel3" noise function, described by [Squirrel Eiserloh](http://www.eiserloh.net/bio/) in [this talk](https://www.youtube.com/watch?v=LWFzPP8ZbdU),
and "Squirrel5" noise function, which is an [improvement](https://twitter.com/SquirrelTweets/status/1421251894274625536?s=20)
over the "Squirrel3" generator.

The `Squirrel3` class, also exposed as `Random`, and the `Squirrel5` class both provide the same
API, as documented below.

## Random

### Static Methods

#### `noise(x: Number): Number`
Given `x` as an integer, this will return a 32-bit number based on the Squirrel3 noise function.

#### `noise(x: Number, seed: Number): Number`
Given `x` and `seed` as integers, this will return a 32-bit number based on the Squirrel3 noise function. The `seed` value can be used to get different outputs for the same position `x`.

### Instance methods
#### `construct new()`
Creates a new instance of a random number generator, seeded based on the current system time.

#### `construct new(seed: Number)`
Creates a new instance of a random number generator, based on the provided seed value.

#### `float(): Number`
Returns a floating point value in the range of `0.0...1.0`, inclusive of `0.0` but exclusive of `1.0`.

#### `float(end: Number): Number`
Returns a floating point value in the range of `0.0...end`, inclusive of `0.0` but exclusive of `end`.

#### `float(start: Number, end: Number): Number`
Returns a floating point value in the range of `start...end`, inclusive of `start` but exclusive of `end`.

#### `int(end: Number): Number`
Returns an integer in the range `0.0...end`, inclusive of `0.0` but exclusive of `end`.
#### `int(start: Number, end: Number): Number`
Returns an integer in the range `start...end`, inclusive of `start` but exclusive of `end`.

#### `sample(list: List): Any`
Given a `list`, this will pick an element from that list at random.

#### `sample(list: List, count: Number): List`
Randomly selects `count` elements from the list and returns them in a new list. This provides "sampling without replacement", so each element is distinct.

#### `shuffle(list: List): List`
Uses the Fisher-Yates algorithm to shuffle the provided `list` in place. The list is also returned for convenience.
