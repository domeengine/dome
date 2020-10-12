#include <ini/ini.h>
#include <ini/ini.c>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"


static int CONFIG_handler(void* config, const char* section, const char* name,
                    const char* value) {

    CONFIG_STORAGE * pconfig = (CONFIG_STORAGE *) config;

    if (pconfig->count > CONFIG_MAX_ITEMS) {
        return -1;
    }

    CONFIG_VALUE item;
    item.section = strdup(section);
    item.name = strdup(name);
    item.value = strdup(value);

    pconfig->items[pconfig->count] = item;
    pconfig->count++;

    return 0;
}

CONFIG_STORAGE CONFIG_readFile(char * string) {
    CONFIG_STORAGE config;
    config.count = 0;

    ini_parse_string(string, CONFIG_handler, &config);
    return config;
}

char * CONFIG_readValue(void* config, char * section, char * name) {
    CONFIG_STORAGE * pconfig = (CONFIG_STORAGE *) config;
    CONFIG_VALUE item;

    for(int i = 0; i < pconfig->count; i++) {
        item = pconfig->items[i];
        if (strcmp(item.section, section) == 0 && strcmp(item.name, name) == 0) {
            return item.value;
        }
    }
    return NULL;
}
