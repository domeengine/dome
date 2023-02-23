// Private string handling methods
// Do not use for game code.
class StringUtils {
  foreign static toLowercase(string)
  foreign static toUppercase(string)

  static subString(str, start, len) {
    return str.bytes.skip(start).take(len).toList
  }
}
