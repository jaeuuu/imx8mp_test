#ifndef API_UART_H_
#define API_UART_H_

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#define DEV_NXP_UART1       "/dev/ttymxc0"
#define DEV_NXP_UART2       "/dev/ttymxc1"
#define DEV_NXP_UART3       "/dev/ttymxc2"
#define DEV_NXP_UART4       "/dev/ttymxc3"

#define BAUD_9600       B9600
#define BAUD_38400      B38400
#define BAUD_115200     B115200

#define MAX_UART_XFER_SIZE  2048

typedef struct st_uart_buf {
    char data[MAX_UART_XFER_SIZE];
    unsigned int    len;
} __attribute__((packed)) uart_buf_t;

typedef struct st_uart {
    char *dev;
    int     baud;
    int     fd;
    sem_t   sem;
} uart_t;

int open_uart(uart_t *uart);
int read_uart(uart_t *uart, char *data, unsigned int len);
int write_uart(uart_t *uart, char *data, unsigned int len);
void close_uart(uart_t *uart);

#endif


