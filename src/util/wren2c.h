#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* WREN2C_readEntireFile(char* path, size_t* lengthPtr);
void WREN2C_encodeAndDump(ENGINE* engine, char* args[]);
