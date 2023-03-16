import "io" for FileSystem

class JsonOptions {
  static nil { 0 }
  static escapeSlashes { 1 }
  static abortOnError { 2 }
  static checkCircular { 4 }

  static contains(options, option) {
    return ((options & option) != JsonOptions.nil)
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
      if (JsonOptions.contains(_options, JsonOptions.abortOnError)) {
        end()
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

class JsonEncoder {
  construct new(options) {
    _options = options
    _circularStack = JsonOptions.contains(options, JsonOptions.checkCircular) ? [] : null
  }

  isCircle(value) {
    if (_circularStack == null) {
      return false
    }
    return _circularStack.any { |v| Object.same(value, v) }
  }

  push(value) {
    if (_circularStack != null) {
      _circularStack.add(value)
    }
  }
  pop() {
    if (_circularStack != null) {
      _circularStack.removeAt(-1)
    }
  }

  encode(value) {
    if (isCircle(value)) {
      Fiber.abort("Circular JSON")
    }

    // Loosely based on https://github.com/brandly/wren-json/blob/master/json.wren
    if (value is Num || value is Bool || value is Null) {
      return value.toString
    }

    if (value is String) {
      // Escape special characters
      var substrings = []
      for (char in value) {
        substrings.add(JsonStream.escapechar(char, _options))
      }

      // Compile error if you use normal escaping sequence
      // so we have to use bytes to string method for the single " char
      return String.fromByte(0x22) + substrings.join("") + String.fromByte(0x22)
    }

    if (value is List) {
      push(value)
      var substrings = []
      for (item in value) {
        substrings.add(encode(item))
      }
      pop()
      return "[" + substrings.join(",") + "]"
    }

    if (value is Map) {
      push(value)
      var substrings = []
      for (key in value.keys) {
        var keyValue = this.encode(value[key])
        var encodedKey = this.encode(key.toString)
        substrings.add("%(encodedKey):%(keyValue)")
      }
      pop()
      return "{" + substrings.join(",") + "}"
    }

    // Default behaviour is to invoke the toString method
    return encode(value.toString)
  }
}

class Json {

  static load(path, options) {
    var content = FileSystem.load(path)
    return Json.decode(content, options)
  }

  static load(path) {
    return Json.load(path, JsonOptions.abortOnError)
  }

  static save(path, object, options) {
    var content = Json.encode(object, options)
    FileSystem.save(path, content)
    return content
  }

  static save(path, object) {
    return Json.save(path, object, JsonOptions.abortOnError)
  }

  static encode(value, options) { JsonEncoder.new(options).encode(value) }

  static encode(value) {
    return Json.encode(value, JsonOptions.abortOnError)
  }

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
    return Json.decode(value, JsonOptions.abortOnError)
  }
}

var JSON = Json
var JSONOptions = JsonOptions
var JSONError = JsonError
