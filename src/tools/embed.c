#include "wrenembed.c"
#include <string.h>

int main(int argc, char* args[])
{
  if (argc < 2) {
    printf("./embed sourceFile [moduleName] [destinationFile]\n");
    printf("Not enough arguments.\n");
    return EXIT_FAILURE;
  }
  
  return WRENEMBED_encodeAndDump(argc, args);
}



