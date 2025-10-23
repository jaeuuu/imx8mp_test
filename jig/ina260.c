#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>

#define INA260_I2C_ADDRESS 0x40
#define INA260_REG_CONFIG     0x00
#define INA260_REG_CURRENT    0x01
#define INA260_REG_VOLTAGE    0x02
#define INA260_REG_POWER      0x03
#define INA260_REG_MASK_ENABLE 0x06
#define INA260_REG_ALERT_LIMIT 0x07
#define INA260_REG_MANUFACTURER_ID 0xFE
#define INA260_REG_DIE_ID     0xFF

int ina260 = -1;

int ina260_write(int fd, int reg, int value)
{
    char buf[3];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msg;

    buf[0] = reg;
    buf[1] = (value >> 8) & 0xFF; // MSB
    buf[2] = value & 0xFF;        // LSB

    msg.addr = INA260_I2C_ADDRESS;
    msg.flags = 0;    // Write
    msg.len = 3;
    msg.buf = buf;

    packet.msgs = &msg;
    packet.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &packet) < 0) {
        perror("ioctl() fail");
        return -1;
    }

    return 0;
}

int ina260_read(int fd, int reg)
{
    char buf[2];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msgs[2];

    buf[0] = reg;

    msgs[0].addr = INA260_I2C_ADDRESS;
    msgs[0].flags = 0;     // Write
    msgs[0].len = 1;
    msgs[0].buf = buf;

    msgs[1].addr = INA260_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD; // Read
    msgs[1].len = 2;
    msgs[1].buf = buf;

    packet.msgs = msgs;
    packet.nmsgs = 2;

    if (ioctl(fd, I2C_RDWR, &packet) < 0) {
        perror("ioctl() fail");
        return -1;
    }

    //*value = (buf[0] << 8) | buf[1];
    return ((buf[0] << 8) | buf[1]);
}

float ina260_get_current(void)
{
    return ina260_read(ina260, INA260_REG_CURRENT) * 1.25 / 1000;
}

float ina260_get_voltage(void)
{
    return ina260_read(ina260, INA260_REG_VOLTAGE) * 1.25 / 1000;
}

float ina260_get_power(void)
{
    return ina260_read(ina260, INA260_REG_POWER) * 10.0 / 1000;
}

int ina260_get_manufacturer_id(void)
{
    return ina260_read(ina260, INA260_REG_MANUFACTURER_ID);
}

int ina260_get_die_id(void)
{
    return ina260_read(ina260, INA260_REG_DIE_ID);
}

int ina260_set_curr_limit_over(int limit_mA)
{
}

int ina260_set_curr_limit_under(int limit_mA)
{
}

int ina260_set_volt_limit_over(int limit_mV)
{
}

int ina260_set_volt_limit_under(int limit_mV)
{
}

int ina260_set_power_limit_over(int limit_mW)
{
}

void ina260_init(void)
{
    ina260 = open("/dev/i2c-3", O_RDWR);

    if (ina260 < 0) {
        perror("open() fail");
        exit(EXIT_FAILURE);
    }
}