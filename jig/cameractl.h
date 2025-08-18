#ifndef CAMERACTL_H_
#define CAMERACTL_H_

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>

int camera_ctl(void);
void camera_init(void);

#endif
