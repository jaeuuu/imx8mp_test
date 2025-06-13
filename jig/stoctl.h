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

typedef struct st_dir_info {
    char path[32];
    DIR *dp;
}dir_info_t;

void sto_control(void);

#endif
