#ifndef TIMER_H__
#define TIMER_H__

#include <time.h>
#include <signal.h>

int set_timer(timer_t *timer, void (*func)(void), int timeout);
int del_timer(timer_t timer);

#endif
