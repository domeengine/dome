#ifdef __MINGW32__
#include <windows.h>
#endif
#include <string.h>
#include "embedlib.c"

internal char*
EMBED_readEntireFile(char* path, size_t* lengthPtr) {
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    return NULL;
  }

  char* source = NULL;
  if (fseek(file, 0L, SEEK_END) == 0) {

    // Get the size of the file.
    long bufsize = ftell(file);

    // Allocate our buffer to that size.
    source = malloc(sizeof(char) * (bufsize + 1));

    // Go back to the start of the file.
    if (fseek(file, 0L, SEEK_SET) != 0) {
      // Error
    }

    // Read the entire file into memory.
    size_t newLen = fread(source, sizeof(char), bufsize, file);

    if (ferror(file) != 0) {
      fclose(file);
      return NULL;
    }

    if (lengthPtr != NULL) {
      *lengthPtr = newLen;
    }

    // Add NULL, Just to be safe.
    source[newLen++] = '\0';
  }

  fclose(file);
  return source;
}

int main(int argc, char* args[])
{
  if (argc < 2) {
    printf("./embed sourceFile [moduleName] [destinationFile]\n");
    printf("Not enough arguments.\n");
    return EXIT_FAILURE;
  }

  size_t length;
  char* fileName = args[1];
  char* fileToConvert = EMBED_readEntireFile(fileName, &length);

  if (fileToConvert == NULL) {
    fputs("Error reading file\n", stderr);
    return EXIT_FAILURE;
  }

  // TODO: Maybe use the filename as a default identifier
  char* moduleName = "wren_module_test";

  if(argc > 2) {
    // TODO: Maybe sanitize moduleName to be valid C identifier?
    moduleName = args[2];
  }

  char* destination = NULL;
  if(argc > 3) {
    destination = args[3];
  } else {
    // Example: main.wren.inc
    destination = strcat(strdup(fileName), ".inc");
  }

  return EMBED_encode(fileToConvert, length, moduleName, destination);
}

