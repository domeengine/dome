#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* WREN2C_hash(char *key)
{
    // https://en.wikipedia.org/wiki/Jenkins_hash_function
    // http://www.burtleburtle.net/bob/hash/doobs.html
    int len = strlen(key);
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    char out[10];
    sprintf(out, "%d", hash);

    return out;
}

char* WREN2C_readEntireFile(char* path, size_t* lengthPtr) {
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


void WREN2C_encodeAndDump(ENGINE* engine, int argc, char* args[]) {
  
  char* fileName = args[1];

  char* name;
  sprintf(name, "%s.inc", fileName);

  char* fileOut = strdup(name); 

  sprintf(name, "wrenModule_%s", WREN2C_hash(fileName));  
  char* moduleName = strdup(name);

  if (argc == 2) {
    moduleName = args[2];
  }

  if (argc == 3) {
    fileOut = args[3];
  }

  size_t length;
  char* fileToConvert = readEntireFile(fileName, &length);

  FILE *fp;
  fp = fopen(fileOut, "w+");
  fputs("// Generated automatically using ./dome --wren2c ", fp);
  fputs(fileName);
  fputs(" Do not modify\n");

  fputs("static const char ", fp);
  fputs(moduleName, fp);

  fputs("[", fp);
  fprintf(fp, "%li", length + 1);
  fputs("] = {", fp);
  
  for (size_t i = 0; i < length; i++ ) {
    char* ptr = fileToConvert + i;
  
    if (*ptr == '\n') {
      fputs("'\\n',", fp);
      fputs("\n", fp);
    } else {
      fputs("'", fp);
      if (*ptr == '\'') {
        fputs("\\\'", fp);
      else if (*ptr == '"') {
        fputs("\\\"", fp);
      } else {
        fwrite(ptr, sizeof(char), 1, fp);
      }
      fputs("', ", fp);
    }
  }
  fputs("\0", fp);

  fputs(" };\n", fp);

  fclose(fp);
  free(fileToConvert);
  free(name);
}
