[< Back](.)

math
=============

The `math` module provides utilities for performing mathematical calculations.

It contains the following classes:

* [Math](#math)
* [Vector](#vector)

## Math

The `Math` class provides various common mathematical functions. For convenience, you can also import this class `M`, like so:

```wren
import "math" for M, Math
System.print(M.min(21, 42))
System.print(Math.max(21, 42))
```

### Static Methods

#### `abs(n: Number): Number`
Returns the absolute magnitude of `n`.

#### `acos(n: Number): Number`
Returns the arccosine of `n`.

#### `asin(n: Number): Number`
Returns the arcsine of `n`.

#### `atan(n: Number): Number`
#### `atan(n1: Number, n2: Number): Number`
Returns the arctan of `n`.

#### `ceil(n: Number): Number`
Rounds `n` up to the next largest integer value.

#### `cos(n: Number): Number`
Returns the cosine of `n`.

#### `floor(n: Number): Number`
Rounds `n` down to the next smallest integer value.

#### `lerp(low: Number, value: Number, high: Number): Number`
This performs a linear interpolation between `low` and `high`, based on the value of `value`, which will be clamped to a range of [0..1].

#### `log(n: Number): Number`
Returns the natural logarithm of `n`.

#### `max(n1: Number, n2: Number): Number`
Returns the larger of `n1` and `n2`.

#### `mid(n1: Number, n2: Number, n3: Number): Number`
Returns the number which is in the middle of the others. This can be used as an inclusive clamping function.

#### `min(n1: Number, n2: Number): Number`
Returns the smaller of `n1` and `n2`.

#### `round(n: Number): Number`
Rounds `n` to the nearest integer value.

#### `sign(n: Number): Number`
If `n` is negative, this returns `-1`. If `n` is positive, this returns `1`. If `n` is equal to zero, it returns `0`.

#### `sin(n: Number): Number`
Returns the sine of `n`.

#### `tan(n: Number): Number`
Returns the tan of `n`.

## Vector

The `Vector` class works as a vector of up to 4 dimensions. You can also refer to it as a `Point` or `Vec`.

### Constructor

#### `Vector.new(): Vector`
#### `Vector.new(x, y): Vector`
#### `Vector.new(x, y, z): Vector`
#### `Vector.new(x, y, z, w): Vector`

Create a vector. If a value isn't provided, it is set to `(0, 0, 0, 0)`.
Unless you specifically need 3 or 4-dimensional vectors, you can ignore _z_ and _w_.

### Instance Fields
#### `x: Number`
#### `y: Number`
#### `z: Number`
#### `w: Number`

#### `manhattan: Number`
This returns the "taxicab length" of the vector, easily calculated as `x` + `y` + `z` + `w`. 

#### `length: Number`
This returns the Euclidean magnitude of the vector.

#### `perp: Vector`
Returns the 2D vector perpendicular to the current vector. This doesn't work for vectors with 3 or more dimensions. 

#### `unit: Vector`
Returns a copy of the current vector, where it's arguments have been scaled such that it's length is 1.


### Instance Methods

#### `dot(vec: Vector): Num`
This returns the dot-product of the vector with another vector.
#### `cross(vec: Vector): Num`
This returns the cross-product of the vector with another vector, resulting in a new vector that is perpendicular to the other two. This only works for 3-dimensional vectors. The _w_ component will be discarded.


### Operators
#### `-Vector: Vector`
Returns the vector, but it's elements are multiplied by -1.

#### `Vector + Vector: Vector`
Returns an element-wise addition of the two Vectors. This will error if you try to add a Vector to something other than a Vector.

#### `Vector - Vector: Vector`
Returns an element-wise subtraction of the two Vectors. This will error if you try to subtract a Vector from something other than a Vector, or vice versa.

#### `Vector * Number: Vector`
Returns the given vector, where it's elements have been multipled by the scalar. This will error if the multiplier is not a number.

#### `Vector / Number: Vector`
Returns the given vector, where it's elements have been divided by the scalar. This will error if the divisor is not a Number.
