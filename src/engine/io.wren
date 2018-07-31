foreign class File {
  construct load(path) {}
  foreign write(path) {}
  foreign append(path) {}

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
