char* readEntireFile(char* path, size_t* lengthPtr) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    return NULL;
  }
  char* source = NULL;
  if (fseek(file, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufsize = ftell(file);
    /* Allocate our buffer to that size. */
    source = malloc(sizeof(char) * (bufsize + 1));

    /* Go back to the start of the file. */
    if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }

    /* Read the entire file into memory. */
    size_t newLen = fread(source, sizeof(char), bufsize, file);
    if ( ferror( file ) != 0 ) {
      fputs("Error reading file", stderr);
    } else {
      if (lengthPtr != NULL) {
        *lengthPtr = newLen;
      }
      source[newLen++] = '\0'; /* Just to be safe. */
    }
  }
  fclose(file);
  return source;
}


