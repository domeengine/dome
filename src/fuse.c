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
  printf("%i\n", argc);

  char* fileName = args[2];
  // TODO: Derive output from input name
#if defined _WIN32 || __MINGW32__
  char* outputFileName = "game.exe";
#else
  char* outputFileName = "game";
#endif
  if (argc == 4) {
    outputFileName = args[3];
  }
  char* binaryPath = getExecutablePath();
  DOME_EGG_HEADER header;
  if (binaryPath != NULL) {
    printf("Fusing...");
    FILE* binary = fopen(binaryPath, "rb");
    FILE* binaryOut = fopen(outputFileName, "wb");
    FILE* egg = fopen(fileName, "rb");
    if (egg == NULL) {
      printf("Error: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
    int c;
    fseek(binary, 0, SEEK_SET);
    while((c = fgetc(binary)) != EOF) {
      fputc(c, binaryOut);
    }
    uint64_t size = sizeof(DOME_EGG_HEADER);
    while((c = fgetc(egg)) != EOF) {
      fputc(c, binaryOut);
      size++;
    }

    memcpy(header.magic1, "DOME", 4);
    memcpy(header.magic2, "DOME", 4);
    header.version = 1;
    header.offset = size;
    fwrite(&header, sizeof(DOME_EGG_HEADER), 1, binaryOut);
    fclose(binaryOut);
    fclose(binary);
    fclose(egg);
    chmod(outputFileName, 0777);
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
