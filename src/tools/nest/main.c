#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("Hello world");
  int c;
  while ((c = getopt (argc, argv, "o:")) != -1) {
    switch(c) {
      case 'o': printf("%s", optarg); break;
      default: abort();
    }
  }

  return 0;
}
