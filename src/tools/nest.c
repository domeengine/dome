typedef enum { MODE_NONE, MODE_CREATE, MODE_EXTRACT } MODE;
bool DOT_FILES = false;

void NEST_usage(ENGINE* engine) {
  ENGINE_printLog(engine, "\nUsage: \n");
  ENGINE_printLog(engine, "  dome nest [options] [--] (<file> | <directory>) ... \n");
  ENGINE_printLog(engine, "\nOptions: \n");
  ENGINE_printLog(engine, "  -c --create               Create a bundle from files.\n");
  ENGINE_printLog(engine, "  -h --help                 Show this help message.\n");
  ENGINE_printLog(engine, "  -o FILE, --output=FILE    The file to bundle into.\n");
  ENGINE_printLog(engine, "  --include-dot-files       Include dot-prefixed files in bundle.\n");
  ENGINE_printLog(engine, "\n");
}

int NEST_writeFile(ENGINE* engine, mtar_t* tar, char* filePath, char* tarPath) {
  size_t length;
  char* inputFile = readEntireFile(filePath, &length);
  if (inputFile == NULL) {
    return EXIT_FAILURE;
  }
  if (tarPath == NULL) {
     tarPath = filePath;
  }
  ENGINE_printLog(engine, "Bundling: %s\n", filePath);
  mtar_write_file_header(tar, tarPath, length);
  mtar_write_data(tar, inputFile, length);
  free(inputFile);
  return EXIT_SUCCESS;
}

int NEST_packDirectory(ENGINE* engine, mtar_t* tar, char* directory, size_t start) {
  struct dirent* de = NULL;
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

      if (!DOT_FILES && strncmp(de->d_name, ".", 1) == 0) {
        continue;
      }

      if (de->d_type == DT_LNK) {
        ENGINE_printLog(engine, "Ignoring symlink: %s\n", path);
      } else if (((de->d_type & DT_DIR) != 0)) {
        // If this is a directory, but not a hidden one
        result = NEST_packDirectory(engine, tar, path, start);
      } else {
        result = NEST_writeFile(engine, tar, path, path + start);
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
    {"create", 'c', OPTPARSE_NONE},
    {"output", 'o', OPTPARSE_REQUIRED},
    {"extract", 'x', OPTPARSE_NONE},
    {"help", 'h', OPTPARSE_NONE},
    {"include-dot-files", 128, OPTPARSE_NONE},
    {0}
  };
  int option;
  MODE mode = MODE_NONE;
  char* outputFileName = "game.egg";
  while ((option = optparse_long(&options, longopts, NULL)) != -1) {
    switch (option) {
      case 128: DOT_FILES = true; break;
      case 'h':
        NEST_usage(engine);
        return EXIT_SUCCESS;
      case 'x':
        if (mode != MODE_NONE) {
          return EXIT_FAILURE;
        }
        mode = MODE_EXTRACT;
        break;
      case 'c':
        if (mode != MODE_NONE) {
          return EXIT_FAILURE;
        }
        mode = MODE_CREATE;
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
    mode = MODE_CREATE;
  }

  bool singleFile = (argc == 1);
  if (mode == MODE_CREATE) {
    mtar_t tar;
    mtar_open(&tar, outputFileName, "w");
    char* path = NULL;
    while ((path = optparse_arg(&options))) {
      /* Is this a file or directory? */
      struct stat info;
      stat(path, &info);
      bool isLink = S_ISLNK(info.st_mode);
      bool isFile = S_ISREG(info.st_mode);
      if (isLink) {
        ENGINE_printLog(engine, "Ignoring symlink: %s\n", path);
        continue;
      } else if (isFile) {
        if (!DOT_FILES && path[0] == '.' && path[1] != '.' ) {
          continue;
        }
        /* File's get added */
        NEST_writeFile(engine, &tar, path, NULL);
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
    ENGINE_printLog(engine, "Created bundle %s.\n", outputFileName);
  }


  return EXIT_SUCCESS;
}
