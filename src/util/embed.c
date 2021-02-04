#include "wren2cinc.c"

int main(int argc, char* args[])
{
  if (argc < 2) {
    printf("./embed sourceFile [moduleName] [destinationFile]\n");
    printf("Not enough arguments.\n");
    return EXIT_FAILURE;
  }
  
  return WREN2CINC_encodeAndDump(argc, args);
}



