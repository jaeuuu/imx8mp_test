#ifndef UARTCTL_H_
#define UARTCTL_H_

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>

#define UART_MXC1       0
#define UART_MXC3       1
#define UART_SERIAL1    2
#define UART_SERIAL2    3
#define UART_SERIAL3    4
#define UART_SERIAL4    5

int uart_ctl(void);
void uart_init(void);

#endif
