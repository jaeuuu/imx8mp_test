#ifndef API_QUEUE_POSIX_H__
#define API_QUEUE_POSIX_H__

#include <mqueue.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int init_que_posix(const char *path, int msize, int dsize);
int recv_que_posix(const char *path, void *buf);
int send_que_posix(const char *path, void *buf);

#endif
