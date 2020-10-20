#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mkdirp.h"

#ifdef _WIN32
#define PATH_SEPARATOR   '\\'
#else
#define PATH_SEPARATOR   '/'
#endif

char *
path_normalize(const char *path) {
    if(!path) return NULL;

    char *copy = strdup(path);
    if(copy == NULL) return NULL;
    char *ptr = copy;

    for (int i = 0; copy[i]; ++i) {
        *ptr++ = path[i];
        if (path[i] == '/') {
            ++i;
            while(path[i] == '/') {
                --i;
            }
        }
    }
    *ptr = '\0';
    return copy;
}

int
mkdirp(const char *path, mode_t mode) {
    char *pathname = NULL;
    char *parent = NULL;

    if(path == NULL) return -1;
    pathname = path_normalize(path);
    if (pathname == NULL) goto fail;

    parent = strdup(pathname);
    if (parent == NULL) goto fail;

    char *p = parent + strlen(parent);
    while(PATH_SEPARATOR != *p && p != parent) {
        --p;
    }
    *p = '\0';

    if(p != parent && mkdir(parent, mode != 0)) goto fail;
    free(parent);

#ifdef _WIN32
    int rc = mkdir(pathname);
#else
    int rc = mkdir(pathname, mode);
#endif

    free(pathname);

    return rc == 0 || EEXIST == errno ? 0 : -1;

fail:
    free(pathname);
    free(parent);
    return -1;
}
