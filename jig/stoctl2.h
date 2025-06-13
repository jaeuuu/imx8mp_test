#ifndef STOCTL_H_
#define STOCTL_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "menu.h"

#define PATH_STORAGE "/run/media/sd"
#define MAX_STO_NUM 5
#define MAX_FILE_NUM 256

#define MAX_STRING_SIZE 128
#define MAX_TMP_SIZE 512

#define F_LIST 0
#define F_READ 1
#define F_CREAT 2
#define F_REMOVE 3
#define F_WRITE 4


typedef struct st_dir_info {
    char path[32];
    char cmd;
    DIR *dp;
} dir_info_t;

typedef struct st_file_info {
    dir_info_t *dirp;
    char fname[128];
    int fnum;
} file_info_t;


void sto_control(void);

#endif
