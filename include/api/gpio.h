#ifndef GPIO_H__
#define GPIO_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <semaphore.h>

#define PATH_DEV_GPIO   "/dev/gio"

#define GPIO_LOW	0
#define GPIO_HIGH	1

#define GPIO_DIR_OUTPUT    0
#define GPIO_DIR_INPUT     1

#define GPIO_PORT1          1
#define GPIO_PORT2          2
#define GPIO_PORT3          3
#define GPIO_PORT4          4
#define GPIO_PORT5          5
#define GPIO_PORT6          6
#define GPIO_PORT7          7

#define GPIO_PIN0           0
#define GPIO_PIN1           1
#define GPIO_PIN2           2
#define GPIO_PIN3           3
#define GPIO_PIN4           4
#define GPIO_PIN5           5
#define GPIO_PIN6           6
#define GPIO_PIN7           7
#define GPIO_PIN8           8
#define GPIO_PIN9           9
#define GPIO_PIN10          10
#define GPIO_PIN11          11
#define GPIO_PIN12          12
#define GPIO_PIN13          13
#define GPIO_PIN14          14
#define GPIO_PIN15          15
#define GPIO_PIN16          16
#define GPIO_PIN17          17
#define GPIO_PIN18          18
#define GPIO_PIN19          19
#define GPIO_PIN20          20
#define GPIO_PIN21          21
#define GPIO_PIN22          22
#define GPIO_PIN23          23
#define GPIO_PIN24          24
#define GPIO_PIN25          25
#define GPIO_PIN26          26
#define GPIO_PIN27          27
#define GPIO_PIN28          28
#define GPIO_PIN29          29
#define GPIO_PIN30          30
#define GPIO_PIN31          31

typedef struct st_gpio_ctrl {
    uint32_t port;
    uint32_t pin;
    uint32_t data;
} __attribute__((packed)) gpio_ctrl_t;

typedef struct st_gpio {
    char *dev;
    int     fd;
    sem_t   sem;
} gpio_t;

int open_gpio(gpio_t *gpio);
int read_gpio(gpio_t *gpio, gpio_ctrl_t *ctrl);
int write_gpio(gpio_t *gpio, gpio_ctrl_t *ctrl);
void close_gpio(gpio_t *gpio);
int set_gpio_direction(gpio_t *gpio, gpio_ctrl_t *ctrl);

#endif


