// Converts a Wren source file to a C include file
// Using standard IO only
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef internal
#define internal static
#endif


internal int
EMBED_encode(char* fileToConvert, size_t length, char* moduleName, char* destinationPath) {
  FILE* fp = fopen(destinationPath, "w+");

  fputs("// auto-generated file, do not modify\n", fp);
  fputs("const char ", fp);
  fputs(moduleName, fp);
  fputs("[", fp);
  fprintf(fp, "%zu", length + 1);
  fputs("] = {", fp);

  // Encode chars
  for (size_t i = 0; i < length; i++ ) {
    char* ptr = fileToConvert + i;
    if (*ptr == '\n') {
      fputs("'\\n',", fp);
      fputs("\n", fp);
    } else {
      fputs("'", fp);

      // TODO: Properly test the encoding with different source files
      if (*ptr == '\'') {
        fputs("\\\'", fp);
      } else if (*ptr == '\\') {
        fputs("\\\\", fp);
      } else {
        fwrite(ptr, sizeof(char), 1, fp);
      }

      fputs("', ", fp);
    }
  }

  fputs("\0", fp);
  fputs(" };\n", fp);

  fclose(fp);

  return EXIT_SUCCESS;
}

