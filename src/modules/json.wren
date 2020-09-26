import "io" for FileSystem

class JsonOptions {
    static NIL_OPTIONS { 0 }
    static ESCAPE_SLASHES { 1 }
    static ABORT_ON_ERROR { 2 }

    static shouldAbort(options) {
        return (options == JsonOptions.ABORT_ON_ERROR || options == (JsonOptions.ABORT_ON_ERROR | JsonOptions.ESCAPE_SLASHES))
    }
}

class Json {

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

    json { _json }
    raw { _raw }
    error { _error }
    options { _options }

    construct parse(string, options) {
        _lastEvent = isInit
        _error = null
        _raw = string
        _options = options

        stream_begin(_raw)
        _json = process_event(next)
        stream_end()
    }

    dumps() {
        return Json.dumps(_json)
    }

    dumps(options) {
        return Json.dumps(_json, options)
    }

    stringify() {
        return Json.dumps(_json)
    }

    stringify(options) {
        return Json.dumps(_json, options)
    }

    save(path) {
        return Json.save(path, _json)
    }

    save(path, options) {
        return Json.save(path, _json, options)
    }

    process_event(event) {

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
                elements.add(process_event(event))
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
                elements[this.value] = process_event(next)
            }
            return elements
        }

        if(event == isError) {
            var error = {}
            
            error["line"] = lineno
            error["pos"] = pos
            error["message"] = error_message

            _error = error
            if(JsonOptions.shouldAbort(_options)) {
                Fiber.abort("JSON Error: line %(lineno) pos %(pos): %(error_message)")
            }
        }
    }

    static parse(string) {
        return Json.parse(string, JsonOptions.NIL_OPTIONS)
    }

    static load(string) {
        return Json.parse(string)
    }

    static load(string, options) {
        return Json.parse(string, options)
    }

    static fromFile(path, options) {
        var content = FileSystem.load(path)
        return Json.parse(content, options)
    }

    static fromFile(path) {
        var content = FileSystem.load(path)
        return Json.parse(content)
    }

    static save(path, obj) {
        return Json.save(path, obj, JsonOptions.NIL_OPTIONS)
    }

    static save(path, obj, options) {
        var content = Json.dumps(obj, options)
        return FileSystem.save(path, content)
    } 

    // Loosely based on https://github.com/brandly/wren-json/blob/master/json.wren
    static dumps(obj, options) {
        if (obj is Num || obj is Bool || obj is Null) {
            return obj.toString
        }
        
        if (obj is String) {
            // Escape special characters
            var substrings = []
            for (char in obj) {
                substrings.add(Json.escapechar(char, options))
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

    static dumps(obj) {
        return Json.dumps(obj, JsonOptions.NIL_OPTIONS)
    }

    static stringify(obj) {
        return Json.dumps(obj)
    }

    static stringify(obj, options) {
        return Json.dumps(obj, options)
    }
}