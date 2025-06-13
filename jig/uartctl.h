#ifndef UARTCTL_H_
#define UARTCTL_H_

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>
#include "menu.h"
#include "api/uart.h"

#define UART_LTE        0
#define UART_WVU        1
#define UART_SOLAR      2
#define UART_WATER1     3
#define UART_WATER2     4

void uart_control(void);
void uart_init(void);
void lte_tx(char *buf, int len);
void wvu_tx(char *buf, int len);
void solar_tx(char *buf, int len);
void water1_tx(char *buf, int len);
void water2_tx(char *buf, int len);

#endif
