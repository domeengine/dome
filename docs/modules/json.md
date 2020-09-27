[< Back](.)

json
================

The `json` module provides a simple interface to read and write [json files and strings](https://www.json.org/json-en.html).

It contains the following classes:

* [Json](#json)
* [JsonOptions](#jsonoptions)
* [JsonError](#jsonerror)

## Json

### Instance Properties

#### `json: Map`
This returns a _Map_ holding the information parsed from the initial json string.

### `options: Num`
The options used when creating this object.

### `raw: String`
This returns the initial _String_ provided before parsing.

### `error: JsonError`
If a parsing error is found this will contain the line, position and error message.

### Instance Methods

### `dumps(): String`
### `dumps(options: Num): String`
This will transform the _json_ property to _String_. With default or custom options. 

### `save(path: String)`
### `save(path: String, options:Num)`
This will execute `dumps()` or `dumps(options)` and then save the result to a file specified in `path`.

### Factory Methods
#### `construct parse(value:String, options:Num): Json`
#### `static parse(value:String): Json`
Returns a new _Json_ object with default or custom options.

#### `static fromFile(path:String): Json`
#### `static fromFile(path:String, options:Num): Json`
Reads the contents of a file in `path` and returns a new _Json_ object with default or custom options.

#### `static dumps(object: Map): String`
#### `static dumps(object: Map, options:Num): String`
Transform the object to a _Json_ encoded string. With default or custom options.

### `static save(path: String, object: Map)`
### `static save(path: String, object:Map, options:Num)`
This will execute `static dumps(object)` or `static dumps(object, options)` and then save the result to a file specified in `path`.

## JsonOptions

### Constants

#### `static NIL: Num`
_Default_ value. No options selected.

#### `static ESCAPE_SLASHES: Num`
This will encode solidus character (`/`). When converting a `Map` object to a `String`. This encoding is optional and is useful when you need to embed `JSON` inside `HTML <script>` tags. By default _DOME_ does not escape slashes.

#### `static ABORT_ON_ERROR: Num`
By default _DOME_ does not _Abort_ when there is a _JSON parsing error_. Use this option when you need to trigger a `Fiber.abort()` on parse error.

### Example

Use [Bitwise OR](https://wren.io/method-calls.html#operators) operator to select multiple options.

```js
Json.parse(myString, 
	JsonOptions.ESCAPE_SLASHES |
	JsonOptions.ABORT_ON_ERROR
)
```

## JsonError

### Instance Properties

#### `line: Num`
Stores the last parsed line number.

#### `position: Num`
Stores the last parsed cursor position.

#### `message: String`
Stores the generated error message.

#### `found: Bool`
Tells if an error was found.
