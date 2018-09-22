#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stddef.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib/microtar/src/microtar.h"
#include "lib/microtar/src/microtar.c"
#include "io.c"

typedef enum { MODE_NONE, MODE_ZIP, MODE_EXTRACT } MODE;

void loadAndWriteFileToTar(mtar_t* tar, char* filePath) {
  size_t length;
  char* inputFile = readEntireFile(filePath, &length);
  mtar_write_file_header(tar, filePath, length);
  mtar_write_data(tar, inputFile, length);
}

void listDir(mtar_t* tar, char* directory, char* subtract) {
    struct dirent *de;  // Pointer for directory entry
    if (subtract == NULL) {
      subtract = directory;
    }

    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(directory);

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while ((de = readdir(dr)) != NULL) {
      if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", directory, de->d_name);
        char* finalFileName = path + strlen(subtract);
        size_t length;
        char* inputFile = readEntireFile(path, &length);
        printf ("Bundling: %s\n", path);
        printf ("subtract: %s\n", subtract);

        if (((de->d_type & DT_DIR) != 0) && (strncmp(de->d_name, ".", 1) != 0)) {
          mtar_write_dir_header(tar, finalFileName);
          listDir(tar, path, subtract);
        } else {
          mtar_write_file_header(tar, finalFileName, length);
          mtar_write_data(tar, inputFile, length);
        }
      }
    }

    closedir(dr);

}

int main(int argc, char **argv) {
  char* tarFileName = NULL;
  int mode = 0;
  int c;
  while ((c = getopt (argc, argv, "vzxo:")) != -1) {
    switch(c) {
      case 'v': printf("NEST v1.0.0\n"); break;
      case 'o': tarFileName = optarg; break;
      case 'z':
                if (mode != 0) { abort(); }
                mode = MODE_ZIP;
                break;
      case 'x':
                if (mode != 0) { abort(); }
                mode = MODE_EXTRACT;
                break;
      default: abort();
    }
  }

  if (mode == MODE_NONE) {
    printf("Error: No mode set.\n");
    return 1;
  } else if (mode == MODE_ZIP && tarFileName != NULL) {
    mtar_t tar;
    /* Open archive for writing */
    mtar_open(&tar, tarFileName, "w");
    printf("%d\n", (argc - optind));
    bool singleFile = (argc - optind) == 1;

    for (int index = optind; index < argc; index++) {
      char path[PATH_MAX];
      snprintf(path, PATH_MAX, "./%s", argv[index]);

      struct stat path_stat;
      stat(path, &path_stat);
      bool isFile = S_ISREG(path_stat.st_mode);
      if (!isFile) {
        listDir(&tar, path, singleFile ? NULL : "");
      } else {
        loadAndWriteFileToTar(&tar, path);
      }
    }
    /* Finalize -- this needs to be the last thing done before closing */
    mtar_finalize(&tar);
    /* Close archive */
    mtar_close(&tar);
    printf("Created bundle %s.\n", tarFileName);
  } else if (tarFileName == NULL) {
    printf("Error: No filename provided.\n");
    return 1;
  }
  return 0;
}
