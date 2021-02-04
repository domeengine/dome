#include "wren2cstring.c"

int main(int argc, char* args[])
{
  if (argc < 2) {
    printf("./embed [WrenFile] [VariableName?] [OutputFile?]\n");
    printf("Not enough arguments.\n");
    return EXIT_FAILURE;
  }
  
  return WREN2CSTRING_encodeAndDump(argc, args, WREN2CSTRING_OUTSIDE_DOME);
}



