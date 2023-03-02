class FileSystem {
  static loadSync(path) {
    System.print("WARN: 'loadSync(_)' is depreciated. Use 'load(_)' instead.")
    return load(path)
  }
  static saveSync(path, buffer) {
    System.print("WARN: 'saveSync(_, _)' is depreciated. Use 'save(_, _)' instead.")
    return save(path, buffer)
  }
  foreign static listFiles(path)
  foreign static listDirectories(path)
  foreign static load(path)
  foreign static save(path, buffer)
  foreign static prefPath(org, app)
  foreign static basePath()
  foreign static createDirectory(path)
  foreign static doesFileExist(path)
  foreign static doesDirectoryExist(path)

  // @Unstable - DO NOT USE
  foreign static f_load(path, op)
  static loadAsync(path) {
    var operation = AsyncOperation.init(null)
    f_load(path, operation)
    return operation
  }

}

foreign class AsyncOperation {
  construct init(empty) {}

  foreign complete
  foreign result

  // foreign error?
}

// Stretchy buffer?
foreign class DataBuffer {
  construct init() {}
  foreign ready
  foreign f_length
  foreign f_data

  length {
    if (ready) {
      return f_length
    } else {
      return null
    }
  }

  data {
    if (ready) {
      return f_data
    } else {
      return null
    }
  }
  // TODO: Index value read and write

  foreign static f_capture()
}
DataBuffer.f_capture()

/*

API Experiment

var operation = FileSystem.write("text.txt", Buffer.text("Hello world"))
if (operation.complete) {
  // do stuff
}


var opertion = FileSystem.read("test.txt");
if (operation.complete) {
  var buffer = operation.data
  System.print(buffer.length)
  System.print(buffer.data)
}

*/
