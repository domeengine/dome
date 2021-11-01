typedef enum { MODE_NONE, MODE_ZIP, MODE_EXTRACT } MODE;

void NEST_usage(ENGINE* engine) {
  // TODO write usage
}

int NEST_writeFile(ENGINE* engine, mtar_t* tar, char* filePath, size_t offset) {
  size_t length;
  char* inputFile = readEntireFile(filePath, &length);
  if (inputFile == NULL) {
    return EXIT_FAILURE;
  }
  ENGINE_printLog(engine, "Bundling: %s\n", filePath);
  mtar_write_file_header(tar, filePath + offset, length);
  mtar_write_data(tar, inputFile, length);
  free(inputFile);
  return EXIT_SUCCESS;
}

int NEST_packDirectory(ENGINE* engine, mtar_t* tar, char* directory, size_t start) {
  struct dirent* de;
  DIR* dr = opendir(directory);
  if (dr == NULL) {
    ENGINE_printLog(engine, "dome: Could not open directory %s\n", directory);
    return EXIT_FAILURE;
  }

  int result;
  while ((de = readdir(dr)) != NULL) {
    // Ignore the current and parent
    result = EXIT_SUCCESS;
    if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
      char path[PATH_MAX];

      strcpy(path, directory);
      strcat(path, "/");
      strcat(path, de->d_name);

      if (strncmp(de->d_name, ".", 1) == 0) {
        continue;
      }

      if (((de->d_type & DT_DIR) != 0)) {
        // If this is a directory, but not a hidden one
        result = NEST_packDirectory(engine, tar, path, start);
      } else {
        result = NEST_writeFile(engine, tar, path, start);
      }
      if (result == EXIT_FAILURE) {
        return EXIT_FAILURE;
      }
    }
  }
  closedir(dr);
  return EXIT_SUCCESS;
}


int NEST_perform(ENGINE* engine, char **argv) {
  struct optparse options;
  optparse_init(&options, argv);
  options.permute = 0;
  struct optparse_long longopts[] = {
    {"help", 'h', OPTPARSE_NONE},
    {"extract", 'x', OPTPARSE_NONE},
    {"bundle", 'z', OPTPARSE_NONE},
    {"output", 'o', OPTPARSE_REQUIRED},
    {0}
  };
  int option;
  MODE mode = MODE_NONE;
  char* outputFileName = "game.egg";
  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 'h':
        NEST_usage(engine);
        return EXIT_SUCCESS;
      case 'x':
        if (mode != MODE_NONE) {
          return EXIT_FAILURE;
        }
        mode = MODE_EXTRACT;
        break;
      case 'z':
        if (mode != MODE_NONE) {
          return EXIT_FAILURE;
        }
        mode = MODE_ZIP;
        break;
      case 'o':
        outputFileName = strdup(options.optarg);
        break;
      case '?':
        ENGINE_printLog(engine, "%s: %s\n", argv[0], options.errmsg);
        return EXIT_FAILURE;
    }
  }
  char** baseArg = argv + options.optind - 1;
  argv += options.optind;

  // Compute the number of things we were told to pack
  char* arg;
  size_t argc = 0;
  while ((arg = optparse_arg(&options)) != NULL) {
    argc++;
  }
  // Reset parser
  optparse_init(&options, baseArg);
  options.permute = 0;

  // For now, we only support ZIP mode so we default to it.
  if (mode == MODE_NONE) {
    mode = MODE_ZIP;
  }

  bool singleFile = (argc == 1);
  if (mode == MODE_ZIP) {
    mtar_t tar;
    mtar_open(&tar, outputFileName, "w");
    char* path = NULL;
    while ((path = optparse_arg(&options))) {
      /* Is this a file or directory? */
      struct stat path_stat;
      stat(path, &path_stat);
      bool isFile = S_ISREG(path_stat.st_mode);
      if (isFile) {
        /* File's get added */
        NEST_writeFile(engine, &tar, path, 0);
      } else {
        /* recurse into directories */
        int last = strlen(path) - 1;
        if (path[last] == '/')
        {
          path[last] = '\0';
        }
        size_t start = singleFile ? strlen(path) + 1 : 0;
        NEST_packDirectory(engine, &tar, path, start);
      }
    }

    /* Finalize -- this needs to be the last thing done before closing */
    mtar_finalize(&tar);
    /* Close archive */
    mtar_close(&tar);
  }

  return EXIT_SUCCESS;
}
