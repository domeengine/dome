#include "tools.h"
#include "embedlib.c"

internal void
EMBED_usage(ENGINE* engine) {
  ENGINE_printLog(engine, "\nUsage: \n");
  ENGINE_printLog(engine, "  dome embed <source file> [<variable>] [<output file>] \n");
  ENGINE_printLog(engine, "  dome embed [options] \n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -h --help    Show this help message.\n");
  ENGINE_printLog(engine, "\n");
}

internal int
EMBED_perform(ENGINE* engine, char** argv) {
  struct optparse options;
  int option;
  optparse_init(&options, argv);
  options.permute = 0;
  struct optparse_long longopts[] = {
    {"help", 'h', OPTPARSE_NONE},
    {0}
  };

  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 'h':
        printTitle(engine);
        EMBED_usage(engine);
        return EXIT_SUCCESS;
      case '?':
        ENGINE_printLog(engine, "dome: %s: %s\n", argv[0], options.errmsg);
        EMBED_usage(engine);
        return EXIT_FAILURE;
    }
  }
  argv += options.optind;

  char* fileName = optparse_arg(&options);
  if (fileName == NULL) {
    ENGINE_printLog(engine, "dome: Missing file name.\n");
    EMBED_usage(engine);
    return EXIT_FAILURE;
  }
  size_t length;
  char* fileToConvert = ENGINE_readFile(engine, fileName, &length, NULL);

  if (fileToConvert == NULL) {
    ENGINE_printLog(engine, "dome: Error reading file: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  char* moduleName = optparse_arg(&options);
  if (moduleName == NULL) {
    moduleName = "wren_module_test";
  }
  char* destination = optparse_arg(&options);
  bool shouldFree = false;
  if (destination == NULL) {
    destination = strdup(fileName);
    destination = realloc(destination, sizeof(char) * (strlen(destination) + 4) + 1);
    strcat(destination, ".inc");
    shouldFree = true;
  }

  int result = EMBED_encode(fileToConvert, length, moduleName, destination);
  free(fileToConvert);
  return result;
}
