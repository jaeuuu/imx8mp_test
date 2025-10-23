#include "i2ciotest.h"
#include <linux/gpio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

static gpiochip_t gpios[MAX_GPIO_PORT];
static gpiostat_t gpio_stat[MAX_GPIO_PORT][MAX_GPIO_PIN];
// static pthread_mutex_t gpio_lock;

static int gpio_read(int fd, int port, int pin)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    req.lineoffsets[0] = pin;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;

    // pthread_mutex_lock(&gpio_lock);
#if 0
    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        // pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
#endif
    if (ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        //close(req.fd);
        // pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    //close(req.fd);
    // pthread_mutex_unlock(&gpio_lock);

    return data.values[0];

}

static int gpio_write(int port, int pin, int sig)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    req.lineoffsets[0] = pin;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;
    data.values[0] = sig;

    // pthread_mutex_lock(&gpio_lock);

    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        // pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
        close(req.fd);
        // pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    close(req.fd);
    // pthread_mutex_unlock(&gpio_lock);

    return data.values[0];
}

static int gpio_config(int port, int pin, int event)
{
    struct gpioevent_request req;

    memset(&req, 0, sizeof(req));
    req.lineoffset = pin;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
    req.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;

    if (ioctl(gpios[port].fd, GPIO_GET_LINEEVENT_IOCTL, &req) < 0) {
        perror("ioctl() ");
        return -1;
    }

    return req.fd;
}

pthread_t ledblk;
static void th_led_blinking(void)
{
    int h, i;

    while (1) {
        for (h = GPIO_PORT6; h < MAX_GPIO_PORT; h++) {
            for (i = 0; i < 8; i++) {
                gpio_write(h, GPIO_PIN8 + i, 1);
            }
        }
        usleep(100 * 1000);

        for (h = GPIO_PORT6; h < MAX_GPIO_PORT; h++) {
            for (i = 0; i < 8; i++) {
                gpio_write(h, GPIO_PIN8 + i, 0);
            }
        }
        usleep(100 * 1000);
    }
}

void gpio_init(void)
{
    int i, port, pin, sig;
    pthread_t det_thread, cpld_det_thread;

    gpios[GPIO_PORT1].dev = PATH_DEV_GPIO1;
    gpios[GPIO_PORT2].dev = PATH_DEV_GPIO2;
    gpios[GPIO_PORT3].dev = PATH_DEV_GPIO3;
    gpios[GPIO_PORT4].dev = PATH_DEV_GPIO4;
    gpios[GPIO_PORT5].dev = PATH_DEV_GPIO5;
    gpios[GPIO_PORT6].dev = PATH_DEV_GPIO6;
    gpios[GPIO_PORT7].dev = PATH_DEV_GPIO7;

    for (i = 0; i < MAX_GPIO_PORT; i++) {
        gpios[i].fd = open(gpios[i].dev, 0);
        if (gpios[i].fd < 0)
            exit(1);
    }

    printf("GPIO Init Ok.\n");
}

int main(void)
{
    int h, i, efd, ret;
    struct pollfd pfds[16];
    struct gpioevent_data ed;

    memset(pfds, 0, sizeof(pfds));
    for (i = 0; i < 16; i++) {
        pfds[i].fd = -1;
    }

    gpio_init();
    for (h = GPIO_PORT6; h < MAX_GPIO_PORT; h++) {
        if (h == GPIO_PORT6) {
            for (i = 0; i < 8; i++) {
                efd = gpio_config(h, i, 0);
                if (efd < 0)
                    return -1;

                pfds[i].fd = efd;
                pfds[i].events = POLLIN;
                pfds[i].revents = 0;
            }
        } else {
            for (i = 8; i < 16; i++) {
                efd = gpio_config(h, i - 8, 0);
                if (efd < 0)
                    return -1;

                pfds[i].fd = efd;
                pfds[i].events = POLLIN;
                pfds[i].revents = 0;
            }
        }
    }

    pthread_create(&ledblk, NULL, (void *)th_led_blinking, NULL);

    while (1) {
        ret = poll(pfds, 16, -1);

        if (ret < 0)
            return -1;

        if (ret == 0)
            continue;

        for (i = 0; i < 16; i++) {
            if (pfds[i].revents & POLLIN) {
                ret = read(pfds[i].fd, &ed, sizeof(ed));
                if (ret != sizeof(ed)) {
                    printf("unmatch size\n");
                    continue;
                }

                if (ed.id == GPIOEVENT_EVENT_FALLING_EDGE ||
                    ed.id == GPIOEVENT_EVENT_RISING_EDGE) {
#if 1
                    if (i < 8) {
                        ret = gpio_read(pfds[i].fd, GPIO_PORT6, i);
                        if (ret < 0) {
                            printf("gpio_read() fail\n");
                            return -1;
                        }

                        printf("event!! [GPIO_PORT6][%d][%s][%s]\n", i,
                            ed.id == GPIOEVENT_EVENT_FALLING_EDGE ? "FALLING" : "RISING",
                            ret ? "HIGH" : "LOW");
                    } else {
                        ret = gpio_read(pfds[i].fd, GPIO_PORT7, i - 8);
                        if (ret < 0) {
                            printf("gpio_read() fail\n");
                            return -1;
                        }

                        printf("event!! [GPIO_PORT7][%d][%s][%s]\n", i - 8,
                            ed.id == GPIOEVENT_EVENT_FALLING_EDGE ? "FALLING" : "RISING",
                            ret ? "HIGH" : "LOW");
                    }
#endif
                }
            }
        }
    }
}


