import "io" for FileSystem

class Version {
  foreign static toString

  static major { this.toList[0] }
  static minor { this.toList[1] }
  static patch { this.toList[2] }

  static toList {
    if (!__list) {
      __list = toString.split(".").map {|value| Num.fromString(value) }.toList
    }
    return __list
  }
  static atLeast(version) {
    var values = version.split(".").map {|value| Num.fromString(value) }.toList
    var actual = this.toList
    if (values[0] > actual[0]) {
      return false
    }
    if (values[0] < actual[0]) {
      return true
    }
    if (values.count > 1) {
      if (values[1] > actual[1]) {
        return false
      }
      if (values[1] < actual[1]) {
        return true
      }
    }
    if (values.count > 2) {
      if (values[2] > actual[2]) {
        return false
      }
      if (values[2] < actual[2]) {
        return true
      }
    }
    return true
  }
}

class Process {
  foreign static f_exit(n)
  static exit(n) {
    f_exit(n)
    Fiber.suspend()
  }

  static exit() {
    exit(0)
  }
}

class Window {
  foreign static title=(value)
  foreign static title
  foreign static vsync=(value)
  foreign static lockstep=(value)
  foreign static fullscreen=(value)
  foreign static fullscreen
  foreign static width
  foreign static height

  foreign static resize(width, height)
}

class Json {

  foreign stream_begin(value)
  foreign stream_end()
  foreign next
  foreign value
  foreign error_message
  foreign lineno
  foreign pos

  foreign static escapechar(value)

  namespace { "com.domeengine:" }
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

  json { _json }
  raw { _raw }
  error { _error }

  construct parse(string) {
    _lastEvent = isInit
    _error = null

    _raw = string
    
    stream_begin(_raw)
    _json = load(next)
    stream_end()
  }

  construct parseFromFile(path) {
    var content = FileSystem.load(path)
    return Json.parse(content)
  }

  load(event) {

    _lastEvent = event

    if(event == isBoolTrue || event == isBoolFalse) {
      return (event == isBoolTrue)
    }

    if(event == isNumeric) {
      return Num.fromString(this.value)
    }

    if(event == isString) {
      return this.value
    }

    if(event == isNull) {
      return null
    }

    if(event == isArray) {
      var elements = []
      while(true) {
        event = next
        _lastEvent = event
        if(event == isArrayEnd || event == isDone || event == isError) {
          break
        }
        elements.add(load(event))
      }
      return elements
    }

    if(event == isObject) {
      var elements = {}
      while(true) {
        event = next
        _lastEvent = event
        if(event == isObjectEnd || event == isDone || event == isError) {
          break
        }
        elements[this.value] = load(next)
      }
      return elements
    }

    if(event == isError) {
      var error = {}
      
      error["line"] = lineno
      error["pos"] = pos
      error["message"] = error_message

      _error = error
    }
  }

  dumps() {
    return Json.dumps(_json)
  }

  // https://github.com/brandly/wren-json/blob/master/json.wren
  static dumps(obj) {
    if (obj is Num || obj is Bool || obj is Null) {
      return obj.toString
    }
    
    if (obj is String) {
      // Escape special characters
      var substrings = []
      for (char in obj) {
        substrings.add(Json.escapechar(char))
      }

      // Compile error if you use normal escaping sequence
      // so we have to use bytes to string method for the single " char
      return String.fromByte(0x22) + substrings.join("") + String.fromByte(0x22)
    }
    
    if (obj is List) {
      var substrings = obj.map { |item| Json.dumps(item) }
      return "[" + substrings.join(",") + "]"
    }
    
    if (obj is Map) {
      var substrings = obj.keys.map { |key|
        return Json.dumps(key) + ":" + Json.dumps(obj[key])
      }
      return "{" + substrings.join(",") + "}"
    }
  }
}