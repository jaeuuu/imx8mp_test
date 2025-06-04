#ifndef API_I2C_H_
#define API_I2C_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <semaphore.h>

#define PATH_DEV_I2C_CH1    "/dev/i2c-0"
#define PATH_DEV_I2C_CH2    "/dev/i2c-1"
#define PATH_DEV_I2C_CH3    "/dev/i2c-2"

enum I2C_ADDRESS_LEN {
    I2C_ADDRESS_NONE = 0,
    I2C_ADDRESS_1BYTE,
    I2C_ADDRESS_2BYTE,
};

// IIC2(Channel1) Chip Address
#define IIC2_TMP100	  0x4A
#define IIC2_EEPROM	  0x53
#define IIC2_RTC	  0x68	  // 수정 요.


typedef struct st_i2c {
    char    *dev;
    int     fd;
    sem_t   sem;
} i2c_t;

#define MAX_I2C_DATA_SIZE   256

typedef struct st_i2c_ctrl {
	unsigned char       slave_addr;
	unsigned short      reg_addr;
	unsigned int        reg_addr_len;
	unsigned char       buf[MAX_I2C_DATA_SIZE];
	unsigned int        buf_len;
}__attribute__((packed)) i2c_ctrl_t;

int open_i2c(i2c_t *i2c);
void close_i2c(i2c_t *i2c);
int write_i2c(i2c_t *i2c, i2c_ctrl_t *ctrl);
int read_i2c(i2c_t *i2c, i2c_ctrl_t *ctrl);

#endif
