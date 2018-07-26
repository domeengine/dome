foreign class File {
  construct load(path) {}
  foreign ready
  foreign f_data

  data {
    if (ready) {
      return f_data
    } else {
      return null
    }
  }
}
