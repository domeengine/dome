/*
 (The MIT License)

Copyright (c) 2013 Stephen Mathieson &lt;me@stephenmathieson.com&gt;

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mkdirp.h"

#ifdef _WIN32
#define PATH_SEPARATOR   '\\'
#else
#define PATH_SEPARATOR   '/'
#endif

char *
path_normalize(const char *path) {
  if (!path) return NULL;

  char *copy = strdup(path);
  if (NULL == copy) return NULL;
  char *ptr = copy;

  for (int i = 0; copy[i]; i++) {
    *ptr++ = path[i];
    if ('/' == path[i]) {
      i++;
      while ('/' == path[i]) i++;
      i--;
    }
  }

  *ptr = '\0';

  return copy;
}


/*
 * Recursively `mkdir(path, mode)`
 */

int
mkdirp(const char *path, mode_t mode) {
  char *pathname = NULL;
  char *parent = NULL;

  if (NULL == path) return -1;

  pathname = path_normalize(path);
  if (NULL == pathname) goto fail;

  parent = strdup(pathname);
  if (NULL == parent) goto fail;

  char *p = parent + strlen(parent);
  while (PATH_SEPARATOR != *p && p != parent) {
    p--;
  }
  *p = '\0';

  // make parent dir
  if (p != parent && 0 != mkdirp(parent, mode)) goto fail;
  free(parent);

  // make this one if parent has been made
  #ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/2fkk4dzw.aspx
    int rc = mkdir(pathname);
  #else
    int rc = mkdir(pathname, mode);
  #endif

  free(pathname);

  return 0 == rc || EEXIST == errno
    ? 0
    : -1;

fail:
  free(pathname);
  free(parent);
  return -1;
}
