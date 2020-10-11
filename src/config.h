
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define CONFIG_MAX_ITEMS 255

typedef struct {
    const char * section;
    const char * name;
    const char * value;
} CONFIG_VALUE;

typedef struct {
    int count;
    CONFIG_VALUE items[CONFIG_MAX_ITEMS];
} CONFIG_STORAGE;

#endif