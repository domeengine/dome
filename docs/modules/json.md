[< Back](.)

json
================

The `json` module provides a simple interface to read and write [json files and strings](https://www.json.org/json-en.html).

It contains the following classes:

* [Json](#json)
* [JsonOptions](#jsonoptions)
* [JsonError](#jsonerror)

## Json

### Static Methods

#### `static encode(object: Object): String`

#### `static encode(object: Object, options:Num): String`

Transform the object to a _Json_ encoded string. With default or custom options.

#### `static decode(value:String, options:Num): Object`

#### `static decode(value:String): Object`

Returns a new _Json_ object with default or custom options.

#### `static load(path:String): Object`

#### `static load(path:String, options:Num): Object`

Reads the contents of a file in `path` and returns a new _Json_ object with default or custom options.

### `static save(path: String, object: Object)`

### `static save(path: String, object:Object, options:Num)`

This will encode the object and then save the result to a file specified in `path`. With default or custom options.

## JsonOptions

### Constants

#### `static nil: Num`

No options selected.

#### `static escapeSlashes: Num`

This will encode solidus character (`/`). When converting a `Map` object to a `String`. This encoding is optional and is useful when you need to embed `JSON` inside `HTML <script>` tags. By default _DOME_ does not escape slashes.

#### `static abortOnError: Num`

By default _DOME_ aborts when there is a _JSON parsing error_ (triggers a `Fiber.abort()` on parse error). Turn off this option if you want to capture the _JsonError_ object.

### Example

Use [Bitwise OR](https://wren.io/method-calls.html#operators) operator to select multiple options.

```js
Json.decode(myString, JsonOptions.escapeSlashes | JsonOptions.abortOnError);
```

## JsonError

This object is returned on calls to _Json.decode_ only if the `JsonOptions.abortOnError` default behaviour is disabled and a parse error was found.

### Instance Properties

#### `line: Num`

Stores the last parsed line number.

#### `position: Num`

Stores the last parsed cursor position.

#### `message: String`

Stores the generated error message.

#### `found: Bool`

Tells if an error was found.
