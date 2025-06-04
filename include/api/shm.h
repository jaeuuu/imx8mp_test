#ifndef API_SHM_H_
#define API_SHM_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "Errors.h"

int init_shm(const char*,int);
int open_shm(const char*,void*,int);
int close_shm(void*,int);

#endif
