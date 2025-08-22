#ifndef LCDCTL_H_
#define LCDCTL_H_

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>

//int uart_ctrl(void);
//void uart_init(void);

int lcd_ctl(void);
void lcd_init(const char *type);

#endif
