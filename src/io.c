internal void FILESYSTEM_loadEventHandler(void* task);

bool doesFileExist(char* path) {
  return access(path, F_OK) != -1;
}

char* readFileFromTar(mtar_t* tar, char* path, size_t* lengthPtr) {
  // We assume the tar open has been done already
  /* Open archive for reading */
  // mtar_t tar;
  // mtar_open(tar, "game.egg", "r");

//

  printf("Reading from tar: %s\n", path);
  mtar_header_t h;
  mtar_find(tar, path, &h);
  size_t length = h.size;
  char* p = calloc(1, length + 1);
  if (mtar_read_data(tar, p, length) != MTAR_ESUCCESS) {
    printf("Error: Couldn't read the data from the bundle.");
    abort();
  }

  if (lengthPtr != NULL) {
    *lengthPtr = length;
  }
  return p;
}

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


