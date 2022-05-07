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

#### `static listDirectories(path: String): List<String>`
Returns a list of all directories contained in the directory

#### `static listFiles(path: String): List<String>`
Returns a list of all files in the directory.

#### `static load(path: String): String`
Given a valid file `path`, this loads the file data into a String object.
This is a blocking operation, and so execution will stop while the file is loaded.

#### `static prefPath(org: String, appName: String): String`
This gives you a safe path where you can write personal game files (saves and settings), which are specific to this application. The given `org` and `appName` should be unique to this application. Either or both values may end up in the given file path, so they must adhere to some specific rules. Use letters, numbers, spaces, underscores. Avoid other punctuation.

#### `static save(path: String, buffer: String): Void`
Given a valid file `path`, this will create or overwrite the file the data in the `buffer` String object.
This is a blocking operation, and so execution will stop while the file is saved.

#### `static createDirectory(path: String): Void`
Given a valid `path`, creates the directory, if it doesn't already exist and makes parent directories as needed.

#### `static doesFileExist(path: String): Boolean`
Checks if the path to the given file exists.

#### `static createDirectory(path: String): Number`
Checks if the path to the given directory exists.
Return values
* 0 means the directory does not exist
* 1 means the directory exists
* 2 means the path leads to A file
