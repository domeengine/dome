#ifdef __MINGW32__
#include <windows.h>
#endif
#include <string.h>
#include "embedlib.c"

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

