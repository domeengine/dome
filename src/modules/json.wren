import "io" for FileSystem

class JsonOptions {
  static NIL { 0 }
  static ESCAPE_SLASHES { 1 }
  static ABORT_ON_ERROR { 2 }

  static shouldAbort(options) {
    return ((options & JsonOptions.ABORT_ON_ERROR) != JsonOptions.NIL)
  }
}

class JsonError {
  line { _line }
  position { _position }
  message { _message }
  found { _found }

  construct new(line, pos, message, found) {
    _line = line
    _position = pos
    _message = message
    _found = found
  }

  static empty() {
    return JsonError.new(0, 0, "", false)
  }
}

class JsonStream {
  foreign stream_begin(value)
  foreign stream_end()
  foreign next
  foreign value
  foreign error_message
  foreign lineno
  foreign pos
  foreign static escapechar(value, options)

  namespace { "com.domeengine.json:" }
  isNull { namespace + "JSON_NULL" }
  isString { namespace + "JSON_STRING" }
  isNumeric { namespace + "JSON_NUMBER" }
  isBoolTrue { namespace + "JSON_TRUE" }
  isBoolFalse { namespace + "JSON_FALSE" }
  isArray { namespace + "JSON_ARRAY" }
  isArrayEnd { namespace + "JSON_ARRAY_END" }
  isObject { namespace + "JSON_OBJECT" }
  isObjectEnd { namespace + "JSON_OBJECT_END" }
  isDone { namespace + "JSON_DONE" }
  isError { namespace + "JSON_ERROR" }
  isInit { namespace + "JSON_INIT" }

  result { _result }
  error { _error }
  options { _options }
  raw { _raw }

  construct new(raw, options) {
    _result = {}
    _error = JsonError.empty()
    _lastEvent = isInit
    _raw = raw
    _options = options
  }

  begin() {
    stream_begin(_raw)
    _result = process(next)
  }

  end() {
    stream_end()
  }

  process(event) {
    _lastEvent = event

    if (event == isError) {
      _error = JsonError.new(lineno, pos, error_message, true)
      if (JsonOptions.shouldAbort(_options)) {
        Fiber.abort("JSON error - line %(lineno) pos %(pos): %(error_message)")
      }
      return
    }

    if (event == isDone) {
      return
    }

    if (event == isBoolTrue || event == isBoolFalse) {
      return (event == isBoolTrue)
    }

    if (event == isNumeric) {
      return Num.fromString(this.value)
    }

    if (event == isString) {
      return this.value
    }

    if (event == isNull) {
      return null
    }

    if (event == isArray) {
      var elements = []
      while (true) {
        event = next
        _lastEvent = event
        if (event == isArrayEnd) {
          break
        }
        elements.add(process(event))
      }
      return elements
    }

    if (event == isObject) {
      var elements = {}
      while (true) {
        event = next
        _lastEvent = event
        if (event == isObjectEnd) {
            break
        }
        elements[this.value] = process(next)
      }
      return elements
    }
  }
}

class Json {

  // MARK: - File IO
  // mirroring the FileSystem class

  /**
  Loads a json file from filesystem.
  - Signature: load(path: String, options:JsonOptions?) -> <Map|List|Num|String|Null|JsonError>
  - Parameter path: The file path where the json file is located.
  - Parameter options: An optional param with options for parsing and encoding json.
  - Returns: A Wren object or JsonError instance.
  - Throws: Fiber.abort() if JsonOption.ABORT_ON_ERROR option is passed.
  */
  static load(path, options) {
    var content = FileSystem.load(path)
    return Json.decode(content, options)
  }

  static load(path) {
    return Json.load(path, JsonOptions.ABORT_ON_ERROR)
  }

  /**
  Saves a Wren object to a json file in filesystem.
  - Signature: save(path: String, object:<String|Num|Bool|Map|List|Null>, options:JsonOptions?) -> String
  - Parameter path: The file path where the json file is located.
  - Parameter object: The Wren object to convert to a json string.
  - Parameter options: An optional param with options for parsing and encoding json.
  - Returns: A json string with the contents of object.
  */
  static save(path, object, options) {
    var content = Json.encode(object, options)
    FileSystem.save(path, content)
    return content
  }

  static save(path, object) {
    return Json.save(path, object, JsonOptions.ABORT_ON_ERROR)
  }

  // MARK: - General Manipulation
  // borrowed from other languages/apis
  /**
  Encodes a Wren object to a String value.
  - Signature: encode(value: <String|Num|Bool|Map|List|Null>, options:JsonOptions?) -> String
  - Parameter value: The Wren object to transform to a json string value.
  - Parameter options: An optional param with options for parsing and encoding json.
  - Returns: String with the contents of the value formatted as a json string.
  */
  static encode(value, options) {
    // Loosely based on https://github.com/brandly/wren-json/blob/master/json.wren
    if (value is Num || value is Bool || value is Null) {
      return value.toString
    }

    if (value is String) {
      // Escape special characters
      var substrings = []
      for (char in value) {
        substrings.add(JsonStream.escapechar(char, options))
      }

      // Compile error if you use normal escaping sequence
      // so we have to use bytes to string method for the single " char
      return String.fromByte(0x22) + substrings.join("") + String.fromByte(0x22)
    }

    if (value is List) {
      var substrings = value.map { |item| Json.encode(item) }
      return "[" + substrings.join(",") + "]"
    }

    if (value is Map) {
      var substrings = value.keys.map { |key|
        return Json.encode(key, options) + ":" + Json.encode(value[key], options)
      }
      return "{" + substrings.join(",") + "}"
    }
  }

  static encode(value) {
    return Json.encode(value, JsonOptions.ABORT_ON_ERROR)
  }

  /**
  Decodes a Wren object from a json string.
  - Signature: decode(value: String, options:JsonOptions?) -> <Map|List|Num|String|Null|JsonError>
  - Parameter value: The json string to be decoded.
  - Parameter options: An optional param with options for parsing and encoding json.
  - Returns: A Wren object or JsonError instance.
  - Throws: Fiber.abort() if JsonOption.ABORT_ON_ERROR option is passed.
  */
  static decode(value, options) {
    var stream = JsonStream.new(value, options)
    stream.begin()

    var result = stream.result
    if (stream.error.found) {
      result = stream.error
    }

    stream.end()
    return result
  }

  static decode(value) {
    return Json.decode(value, JsonOptions.ABORT_ON_ERROR)
  }
}

var JSON = Json
