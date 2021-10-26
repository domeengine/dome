#ifndef __EMSCRIPTEN__

typedef struct {
  FILE* fd;
  size_t offset;
} FUSE_STREAM;


internal char*
getExecutablePath() {
  char* path = NULL;
  size_t size = wai_getExecutablePath(NULL, 0, NULL);
  if (size > 0) {
    path = malloc(size + 1);
    if (path != NULL) {
      wai_getExecutablePath(path, size, NULL);
      path[size] = '\0';
    }
  }
  return path;
}

int fuse(int argc, char* args[])
{
  if (argc < 2) {
    fputs("Not enough arguments\n", stderr);
    return EXIT_FAILURE;
  }

  char* fileName = args[2];
  char* binaryPath = getExecutablePath();
  if (binaryPath != NULL) {
    // Check if end of file has marker

    FILE* binary = fopen(binaryPath, "ab");
    int result = fseek (binary, -sizeof(DOME_EGG_HEADER), SEEK_END);
    DOME_EGG_HEADER header;
    result = fread(&header, sizeof(DOME_EGG_HEADER), 1, binary);
    fclose(binary);

    if (result == 1) {
      if (strncmp("DOME", header.magic2, 4) == 0) {
        printf("This copy of DOME is already fused to an EGG file. Please use a fresh instance.");
        return EXIT_FAILURE;
      }
    }
    printf("Fusing...");
    binary = fopen(binaryPath, "ab");
    FILE* egg = fopen(fileName, "rb");
    if (egg == NULL) {
      printf("Error: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
    int c;
    uint64_t size = sizeof(DOME_EGG_HEADER);
    while((c = fgetc(egg)) != EOF) {
      fputc(c, binary);
      size++;
    }

    strncpy(header.magic1, "DOME", 4);
    strncpy(header.magic2, "DOME", 4);
    header.version = 1;
    header.offset = size;
    fwrite(&header, sizeof(DOME_EGG_HEADER), 1, binary);
    fclose(binary);
    fclose(egg);
  }
  free(binaryPath);
  return EXIT_SUCCESS;
}

int FUSE_read(mtar_t* tar, void *data, unsigned size) {
  FUSE_STREAM* stream = tar->stream;
  unsigned res = fread(data, 1, size, stream->fd);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

int FUSE_seek(mtar_t* tar, unsigned pos) {
  FUSE_STREAM* stream = tar->stream;
  assert(pos <= stream->offset);
  int res = fseek(stream->fd, pos - stream->offset, SEEK_END);
  return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

int FUSE_close(mtar_t* tar) {
  FUSE_STREAM* stream = tar->stream;
  if (stream->fd != NULL) {
    fclose(stream->fd);
  }
  free(stream);
  tar->stream = NULL;
  return MTAR_ESUCCESS;
}

int fuse_open(mtar_t* tar, FILE* fd, size_t offset) {
  FUSE_STREAM* stream = malloc(sizeof(FUSE_STREAM));
  if (stream == NULL) {
    return MTAR_EFAILURE;
  }
  stream->fd = fd;
  stream->offset = offset;

  tar->stream = stream;
  tar->read = FUSE_read;
  tar->seek = FUSE_seek;
  tar->close = FUSE_close;
  tar->write = NULL;

  return MTAR_ESUCCESS;
}

#endif
