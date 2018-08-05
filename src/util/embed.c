//Using SDL and standard IO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readEntireFile(char* path, size_t* lengthPtr) {
  FILE* file = fopen(path, "r");
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

int main(int argc, char* args[])
{
  if (argc != 4) {
    printf("./embed [WrenFile] [VariableName] [OutputFile]\n");
    printf("Not enough arguments.\n");
    return EXIT_FAILURE;
  } 
  size_t length;
  char* fileToConvert = readEntireFile(args[1], &length);
  char* moduleName = args[2];
  FILE *fp;
  fp = fopen(args[3], "w+");
  fputs("// auto-generated file, do not modify\n", fp);
  fputs("const char* ", fp);
  fputs(moduleName, fp);
  fputs(" = \"", fp);
  for (size_t i = 0; i < length; i++ ) {
    char* ptr = fileToConvert + i;
    if (*ptr == '\"') {
      fputs("\\\"", fp);
    } else if (*ptr == '\n') {
      fputs("\\n\"", fp);
      fputs("\n", fp);
      fputs("\"", fp);
    } else {
      fwrite(ptr, sizeof(char), 1, fp);
    }
  }
  
  fputs("\";\n", fp);
  
  fclose(fp);
  free(fileToConvert);
  return EXIT_SUCCESS;
}



