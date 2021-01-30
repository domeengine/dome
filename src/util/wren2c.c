
internal 
void WREN2C_randomString(char *dest) {
    // https://stackoverflow.com/a/15768317
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    srand(time(0));

    size_t length = 10;
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


internal 
void WREN2C_encodeAndDump(ENGINE* engine, int argc, char* args[]) {
  
  char* fileName = args[2];
  char* fileOut = strcat(strdup(fileName), ".inc");

  char randName[] = "";
  WREN2C_randomString(randName);

  char* moduleName = randName;

  // Specify custom module name
  if (argc == 4) {
    moduleName = args[3];
  }

  // Specify custom fileOut name
  if (argc == 5) {
    fileOut = args[4];
  }

  size_t length;

  // readEntireFile is located in io.c
  char* fileToConvert = readEntireFile(fileName, &length);

  FILE *fp;
  fp = fopen(fileOut, "w+");
  fputs("// Generated automatically using ./dome --wren2c ", fp);
  fputs(fileName, fp);
  fputs(" Do not modify\n", fp);

  fputs("const char wrenModule_", fp);
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
}
