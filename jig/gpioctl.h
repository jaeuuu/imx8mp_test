#ifndef GPIOCTL_H_
#define GPIOCTL_H_

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <pthread.h>
#include <stdbool.h>
#include <api/menu.h>

typedef struct st_gpiochip {
    char *dev;
    int fd;
} gpiochip_t;

typedef struct st_gpiostat {
    bool init_flag;
    int inval;
    int outval;
} gpiostat_t;

#define PATH_DEV_GPIO1  "/dev/gpiochip0"
#define PATH_DEV_GPIO2  "/dev/gpiochip1" 
#define PATH_DEV_GPIO3  "/dev/gpiochip2"
#define PATH_DEV_GPIO4  "/dev/gpiochip3"
#define PATH_DEV_GPIO5  "/dev/gpiochip4"
#define PATH_DEV_GPIO6  "/dev/gpiochip5"
#define PATH_DEV_GPIO7  "/dev/gpiochip6"

#define GPIO_PORT1  0
#define GPIO_PORT2  1
#define GPIO_PORT3  2
#define GPIO_PORT4  3
#define GPIO_PORT5  4
#define GPIO_PORT6  5
#define GPIO_PORT7  6
#define MAX_GPIO_PORT 7

#define GPIO_PIN0   0
#define GPIO_PIN1   1
#define GPIO_PIN2   2
#define GPIO_PIN3   3
#define GPIO_PIN4   4
#define GPIO_PIN5   5
#define GPIO_PIN6   6
#define GPIO_PIN7   7
#define GPIO_PIN8   8
#define GPIO_PIN9   9
#define GPIO_PIN10  10
#define GPIO_PIN11  11
#define GPIO_PIN12  12
#define GPIO_PIN13  13
#define GPIO_PIN14  14
#define GPIO_PIN15  15
#define GPIO_PIN16  16
#define GPIO_PIN17  17
#define GPIO_PIN18  18
#define GPIO_PIN19  19
#define GPIO_PIN20  20
#define GPIO_PIN21  21
#define GPIO_PIN22  22
#define GPIO_PIN23  23
#define GPIO_PIN24  24
#define GPIO_PIN25  25
#define GPIO_PIN26  26  
#define GPIO_PIN27  27
#define GPIO_PIN28  28
#define GPIO_PIN29  29
#define GPIO_PIN30  30
#define GPIO_PIN31  31
#define MAX_GPIO_PIN 32

#define MAX_GPIO_DEV    7

int gpio_in_ctl(void);
int gpio_out_ctl(void);
void gpio_init(void);

#endif
