// JSON Parser Events
const char json_typename[][64] = {
    [JSON_ERROR]      = "com.domeengine.json:JSON_ERROR",
    [JSON_DONE]       = "com.domeengine.json:JSON_DONE",
    [JSON_OBJECT]     = "com.domeengine.json:JSON_OBJECT",
    [JSON_OBJECT_END] = "com.domeengine.json:JSON_OBJECT_END",
    [JSON_ARRAY]      = "com.domeengine.json:JSON_ARRAY",
    [JSON_ARRAY_END]  = "com.domeengine.json:JSON_ARRAY_END",
    [JSON_STRING]     = "com.domeengine.json:JSON_STRING",
    [JSON_NUMBER]     = "com.domeengine.json:JSON_NUMBER",
    [JSON_TRUE]       = "com.domeengine.json:JSON_TRUE",
    [JSON_FALSE]      = "com.domeengine.json:JSON_FALSE",
    [JSON_NULL]       = "com.domeengine.json:JSON_NULL",
};

json_stream jsonStream[1];

internal void
JSON_STREAM_begin(WrenVM * vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "value");
  const char * value = wrenGetSlotString(vm, 1);
  json_open_string(jsonStream, value);
  json_set_streaming(jsonStream, 0);
}

internal void
JSON_STREAM_end(WrenVM * vm) {
  json_reset(jsonStream);
  json_close(jsonStream);
}

internal void
JSON_STREAM_value(WrenVM * vm) {
  const char * value = json_get_string(jsonStream, 0);
  wrenSetSlotString(vm, 0, value);
}

internal void
JSON_STREAM_error_message(WrenVM * vm) {
  const char * error = json_get_error(jsonStream);
  if (error) {
    wrenSetSlotString(vm, 0, error);
    return;
  }
  wrenSetSlotString(vm, 0, "");
}

internal void
JSON_STREAM_lineno(WrenVM * vm) {
  wrenSetSlotDouble(vm, 0, json_get_lineno(jsonStream));
}

internal void
JSON_STREAM_pos(WrenVM * vm) {
  wrenSetSlotDouble(vm, 0, json_get_position(jsonStream));
}

internal void
JSON_STREAM_next(WrenVM * vm) {
  enum json_type type = json_next(jsonStream);
  if (type >= JSON_ERROR && type <= JSON_NULL) {
    wrenSetSlotString(vm, 0, json_typename[type]);
    return;
  }
  wrenSetSlotNull(vm, 0);
}

// We have to use C functions for escaping chars
// because a bug in compiler throws error when using \ in strings
enum JSON_DOME_OPTIONS {
    JSON_DOME_NIL_OPTIONS = 0,
    JSON_DOME_ESCAPE_SLASHES = 1,
    JSON_DOME_ABORT_ON_ERROR = 2
};

internal void
JSON_STREAM_escapechar(WrenVM * vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "value");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "options");

  const char * value = wrenGetSlotString(vm, 1);
  int options = (int) wrenGetSlotDouble(vm, 2);

  const char * result = value;

  /*
  "\0" // The NUL byte: 0.
  "\"" // A double quote character.
  "\\" // A backslash.
  "\a" // Alarm beep. (Who uses this?)
  "\b" // Backspace.
  "\f" // Formfeed.
  "\n" // Newline.
  "\r" // Carriage return.
  "\t" // Tab.
  "\v" // Vertical tab.
  */
  if (utf8cmp(value, "\0") == 0) {
    result = "\\0";
  } else if (utf8cmp(value, "\"") == 0) {
    result = "\\\"";
  } else if (utf8cmp(value, "\\") == 0) {
    result = "\\\\";
  } else if (utf8cmp(value, "\\") == 0) {
    result = "\\a";
  } else if (utf8cmp(value, "\b") == 0) {
    result = "\\b";
  } else if (utf8cmp(value, "\f") == 0) {
    result = "\\f";
  } else if (utf8cmp(value, "\n") == 0) {
    result = "\\n";
  } else if (utf8cmp(value, "\r") == 0) {
    result = "\\r";
  } else if (utf8cmp(value, "\t") == 0) {
    result = "\\t";
  } else if (utf8cmp(value, "\v") == 0) {
    result = "\\v";
  } else if (utf8cmp(value, "/") == 0) {
    // Escape / (solidus, slash)
    // https://stackoverflow.com/a/9735430
    // The feature of the slash escape allows JSON to be embedded in HTML (as SGML) and XML.
    // https://www.w3.org/TR/html4/appendix/notes.html#h-B.3.2
    // This is optional escaping. Disabled by default.
    // use JsonOptions.escapeSlashes option to enable it
    if ((options & JSON_DOME_ESCAPE_SLASHES) != JSON_DOME_NIL_OPTIONS) {
        result = "\\/";
    }
  }

  wrenSetSlotString(vm, 0, result);
}
