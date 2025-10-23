#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>

#define PCA6416_I2C_ADDRESS 0x20
#define PCA6416_REG_INPUT_PORT_0  0x00
#define PCA6416_REG_INPUT_PORT_1  0x01
#define PCA6416_REG_OUTPUT_PORT_0 0x02
#define PCA6416_REG_OUTPUT_PORT_1 0x03
#define PCA6416_REG_POLARITY_INV_0 0x04
#define PCA6416_REG_POLARITY_INV_1 0x05
#define PCA6416_REG_CONFIG_0      0x06
#define PCA6416_REG_CONFIG_1      0x07

int pca6416 = -1;

int pca6416_write(int fd, int reg, int value)
{
    char buf[3];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msg;

    buf[0] = reg;
    buf[1] = value & 0xFF;

    msg.addr = PCA6416_I2C_ADDRESS;
    msg.flags = 0;    // Write
    msg.len = 2;
    msg.buf = buf;

    packet.msgs = &msg;
    packet.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &packet) < 0) {
        perror("ioctl() fail");
        return -1;
    }

    return 0;
}

int pca6416_bulk_write(int fd, int reg, int value)
{
    char buf[3];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msg;

    buf[0] = reg;
    buf[1] = value & 0xFF; // MSB
    buf[2] = (value >> 8) & 0xFF;        // LSB

    msg.addr = PCA6416_I2C_ADDRESS;
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

int pca6416_read(int fd, int reg)
{
    char buf[2];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msgs[2];

    buf[0] = reg;

    msgs[0].addr = PCA6416_I2C_ADDRESS;
    msgs[0].flags = 0;     // Write
    msgs[0].len = 1;
    msgs[0].buf = buf;

    msgs[1].addr = PCA6416_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD; // Read
    msgs[1].len = 1;
    msgs[1].buf = buf;

    packet.msgs = msgs;
    packet.nmsgs = 2;

    if (ioctl(fd, I2C_RDWR, &packet) < 0) {
        perror("ioctl() fail");
        return -1;
    }

    //*value = (buf[0] << 8) | buf[1];
    return (buf[0]);
}

int pca6416_bulk_read(int fd, int reg)
{
    char buf[3];
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg msgs[2];

    buf[0] = reg;

    msgs[0].addr = PCA6416_I2C_ADDRESS;
    msgs[0].flags = 0;     // Write
    msgs[0].len = 1;
    msgs[0].buf = buf;

    msgs[1].addr = PCA6416_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD; // Read
    msgs[1].len = 3;
    msgs[1].buf = buf;

    packet.msgs = msgs;
    packet.nmsgs = 2;

    if (ioctl(fd, I2C_RDWR, &packet) < 0) {
        perror("ioctl() fail");
        return -1;
    }

    //*value = (buf[0] << 8) | buf[1];
    return ((buf[0] << 8) | buf[2]);
    //return ((buf[0] << 16) | (buf[1] << 8) | buf[2]);
}

void pca6416_init(void)
{
    pca6416 = open("/dev/i2c-2", O_RDWR);

    if (pca6416 < 0) {
        perror("open() fail");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    int ret;

    pca6416_init();

    // Set all pins of port 0 and port 1 as output
    // pca6416_write(pca6416, PCA6416_REG_CONFIG_0, 0x00);
    // pca6416_write(pca6416, PCA6416_REG_CONFIG_1, 0xff);
    pca6416_bulk_write(pca6416, PCA6416_REG_CONFIG_0, 0x00ff);

    while (1) {

#if 0
        ret = pca6416_bulk_read(pca6416, PCA6416_REG_INPUT_PORT_0);

        printf("\n=============================\n");
        for (int i = 0; i < 16; i++) {
            if (ret & (1 << i))
                printf("PCA6416 PIN[%02d] : HIGH\n", i);
            else
                printf("PCA6416 PIN[%02d] : LOW\n", i);
        }
        printf("=============================\n");
#else

        ret = pca6416_bulk_read(pca6416, PCA6416_REG_INPUT_PORT_0);
        printf("\n-----------------------------\n");
        for (int i = 0; i < 16; i++) {
            if (ret & (1 << i))
                printf("PCA6416 PIN[%02d] : HIGH\n", i);
            else
                printf("PCA6416 PIN[%02d] : LOW\n", i);
        }
        printf("-----------------------------\n");
        sleep(5);
#if 0
        ret = pca6416_read(pca6416, PCA6416_REG_INPUT_PORT_1);
        printf("\n-----------------------------\n");
        for (int i = 0; i < 8; i++) {
            if (ret & (1 << i))
                printf("PCA6416 PIN[%02d] : HIGH\n", i + 8);
            else
                printf("PCA6416 PIN[%02d] : LOW\n", i + 8);
        }
        printf("-----------------------------\n");
        sleep(5);
#endif
#endif

#if 0
        // Set all pins of port 0 and port 1 to high
        pca6416_bulk_write(pca6416, PCA6416_REG_OUTPUT_PORT_0, 0xFFFF);
        // pca6416_write(pca6416, PCA6416_REG_OUTPUT_PORT_1, 0x00);
        sleep(2);
        // Set all pins of port 0 and port 1 to low
        pca6416_bulk_write(pca6416, PCA6416_REG_OUTPUT_PORT_0, 0x0000);
        // pca6416_write(pca6416, PCA6416_REG_OUTPUT_PORT_1, 0xff);
        sleep(2);
#endif
    }

    close(pca6416);
    return 0;
}