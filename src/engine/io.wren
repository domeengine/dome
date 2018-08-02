class FileSystem {
  foreign static f_load(path, op)
  // foreign static f_write_buffer(path, buffer, op) {}
  // foreign static f_write_string(path, str, op) {}
  // foreign f_append(path, buffer, op) {}

  static load(path) {
    var operation = AsyncOperation.init(DataBuffer.create())
    f_load(path, operation)
    return operation
  }

  /*
  // Overwrites entire path
  static write(path, buffer) {
    operation = AsyncOperation.init()
    operation.create(DataBuffer.create())
    if (buffer is String) {
      f_write_string(path, buffer, operation)
    } else if (buffer is DataBuffer) {
      f_write_buffer(path, buffer, operation)
    } else {
      Fiber.abort("Attempted to write unknown buffer type")
    }
    return operation
  }
  */

  // Append

}

foreign class AsyncOperation {
  construct init(result) {}

  foreign complete
  foreign result

  // foreign error?
}

// Stretchy buffer?
foreign class DataBuffer {
  construct create() {}
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
}

/*
// Deprecated
foreign class File {
  construct load(path) {}
  // foreign write(path) {}
  // foreign append(path) {}

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
}

*/

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
