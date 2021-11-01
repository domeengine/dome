int NEST_perform(char **argv) {
  struct optparse options;
  optparse_init(&options, argv);
  options.permute = 0;
  struct optparse_long longopts[] = {
    {"help", 'h', OPTPARSE_NONE},
  };
  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 'h':
        return EXIT_SUCCESS;
      case '?':
        fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
        return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
