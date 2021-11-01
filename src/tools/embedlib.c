// Converts a Wren source file to a C include file
// Using standard IO only
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


char* EMBED_readEntireFile(char* path, size_t* lengthPtr)
{
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

int EMBED_encode(char* fileToConvert, size_t length, char* moduleName, char* destinationPath) {
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

