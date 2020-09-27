[< Back](.)

json
================

The `json` module provides a simple interface to read and write [json files and strings](https://www.json.org/json-en.html).

It contains the following classes:

* [Json](#json)
* [JsonOptions](#jsonoptions)
* [JsonError](#jsonerror)

## Json

### Constructor

#### `constructor parse(value:String, options:Num)`
This returns a new `Json` object .

### Instance Properties

#### `json: Map`
This returns the _Map_ holding the information parsed from the initial json string.

### `options: Num`
The options used when creating this object.

### `raw: String`
This returns the initial _String_ provided before parsing.

### `error: JsonError`
If a parsing error is found this will contain the line, position and error message.

### Instance Methods

### `dumps(): String`
This will transform the _json_ property to _String_. with default options.

### `dumps(options: Num): String`
This will transform the _json_ property to _String_. With custom options. 

### `save(path: String)`
This will execute `dumps()` and then save the result to a file specified in `path`.

### `save(path: String, options:Num)`
This will execute `dumps(options)` and then save the result to a file specified in `path`.

### Static Methods

#### `static parse(value:String): Json`
Returns a new _Json_ object with default options.

#### `static fromFile(path:String): Json`
Reads the contents of a file in `path` and returns a new _Json_ object with default options.

#### `static fromFile(path:String, options:Num): Json`
Reads the contents of a file in `path` and returns a new _Json_ object with custom options.

#### `static dumps(object: Map): String`
Transform the object to a _Json_ encoded string. With default options.

#### `static dumps(object: Map, options:Num): String`
Transform the object to a _Json_ encoded string. With custom options.

### `static save(path: String, object: Map)`
This will execute `static dumps(object)` and then save the result to a file specified in `path`.

### `static save(path: String, object:Map, options:Num)`
This will execute `static dumps(object, options)` and then save the result to a file specified in `path`.

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
