[< Back](.)

io
================

The `io` module provides an interface to the host machine's file system, in order to read and write data files.

It contains the following classes:

* [FileSystem](#filesystem)

## FileSystem

### Static Methods

#### `static basePath(): String`
This returns the path to the directory where your application's entry point is.

#### `static prefPath(org: String, appName: String): String`
This gives you a safe path where you can write personal game files (saves and settings), which are specific to this application. The given `org` and `appName` should be unique to this application. Either or both values may end up in the given file path, so they must adhere to some specific rules. Use letters, numbers, spaces, underscores. Avoid other punctuation.

#### `static load(path: String): String`
Given a valid file `path`, this loads the file data into a String object.
This is a blocking operation, and so execution will stop while the file is loaded.

#### `static save(path: String, buffer: String): Void`
Given a valid file `path`, this will create or overwrite the file the data in the `buffer` String object.
This is a blocking operation, and so execution will stop while the file is saved.
