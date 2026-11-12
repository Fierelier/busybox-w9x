#ifndef BB_TCC_STRING_H
#define BB_TCC_STRING_H
#include_next <string.h>
size_t strnlen(const char *, size_t);
char *strtok_r(char *, const char *, char **);
#endif
