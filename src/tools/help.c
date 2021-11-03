#include "tools.h"

internal void
HELP_usage(ENGINE* engine) {
  ENGINE_printLog(engine, "\nUsage: \n");
  ENGINE_printLog(engine, "  dome help (-h | --help) | (<command>)\n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -h --help    Show this help message.\n");
  ENGINE_printLog(engine, "\n");
}

internal int
HELP_perform(ENGINE* engine, char **argv) {
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
        HELP_usage(engine);
        return EXIT_SUCCESS;
      case '?':
        ENGINE_printLog(engine, "dome: %s: %s\n", argv[0], options.errmsg);
        printUsage(engine);
        return EXIT_FAILURE;
    }
  }
  argv += options.optind;

  char* command = optparse_arg(&options);
  if (command == NULL) {
    ENGINE_printLog(engine, "dome: command is missing.\n");
    HELP_usage(engine);
    return EXIT_FAILURE;
  }
  if (STRINGS_EQUAL(command, "help")) {
    printTitle(engine);
    HELP_usage(engine);
  } else if (STRINGS_EQUAL(command, "fuse")) {
    printTitle(engine);
    FUSE_usage(engine);
  } else if (STRINGS_EQUAL(command, "embed")) {
    printTitle(engine);
    EMBED_usage(engine);
  } else if (STRINGS_EQUAL(command, "nest")) {
    printTitle(engine);
    NEST_usage(engine);
  } else {
    ENGINE_printLog(engine, "dome: command %s was invalid.\n", command);
    printUsage(engine);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
