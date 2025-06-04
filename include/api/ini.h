#ifndef INI_H__
#define INI_H__

#include <stdio.h>
#include <string.h>

#define MAX_INI_STRING_SIZE     64

int create_init(const char *filename);
int read_ini(const char *filename, const char *sec, const char *key, char *buf, unsigned int size);
int write_ini(const char *filename, const char *sec, const char *key, const char *buf, unsigned int size);

#endif
