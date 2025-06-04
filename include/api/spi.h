#ifndef API_SPI_H_
#define API_SPI_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <semaphore.h>

typedef struct st_spi {
    char            *dev;
    unsigned char   mode;
    unsigned char   bits;
    unsigned char   delay;
    unsigned char   speed;
    int             fd;
    sem_t           sem;
} spi_t;

#define MAX_SPI_DATA_SIZE   4096

typedef struct st_spi_ctrl {
    char            tx_buf[MAX_SPI_DATA_SIZE];
    char            rx_buf[MAX_SPI_DATA_SIZE];
    unsigned int    buf_len;
}__attribute__((packed)) spi_ctrl_t;


int open_spi(spi_t *spi);
void close_spi(spi_t *spi);
int write_spi(spi_t *spi, spi_ctrl_t *ctrl);
int read_spi(spi_t *spi, spi_ctrl_t *ctrl);

#endif
