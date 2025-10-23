#include "gpioctl.h"
#include <linux/gpio.h>
#include <ncurses.h>

#define MAX_GPIO_MENU_DEPTH     3

static gpiochip_t gpios[MAX_GPIO_PORT];
static gpiostat_t gpio_stat[MAX_GPIO_PORT][MAX_GPIO_PIN];
static int gpio_input_monitoring;
static pthread_mutex_t gpio_lock;
static WINDOW *pr_win_gpio[MAX_GPIO_MENU_DEPTH];
static int pr_win_gpio_depth = 0;

static int back_on_gpio(void *arg);

static int gpio_read(int port, int pin)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    req.lineoffsets[0] = pin;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;

    pthread_mutex_lock(&gpio_lock);

    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        close(req.fd);
        pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    close(req.fd);
    pthread_mutex_unlock(&gpio_lock);

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

    pthread_mutex_lock(&gpio_lock);

    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
        close(req.fd);
        pthread_mutex_unlock(&gpio_lock);
        return -1;
    }
    close(req.fd);
    pthread_mutex_unlock(&gpio_lock);

    return data.values[0];
}

static int __gpio_input_monitor(void *args)
{
    int *onoff = (int *)args;
    int x, y;

    gpio_input_monitoring = *onoff;

    //getyx(stdscr, y, x);
    //mvprintw(y, 0, "[GPIO INPUT MONITORING]: %s\n", gpio_input_monitoring ? "ON" : "OFF");
    //refresh();
    pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO INPUT MONITORING]: %s\n", gpio_input_monitoring ? "ON" : "OFF");
    return 0;
}

int gpio_input_monitor(void)
{
    int on = 1, off = 0;
    menu_args_t input_monitor_menus[] = {
        //{__gpio_input_monitor, "ON", &on},
        //{__gpio_input_monitor, "OFF", &off},
        {back_on_gpio, "back", &on}
    };
    char *des = "GPIO INPUT MONITORING";
    //pr_win_gpio_depth++;
    gpio_input_monitoring = 1;
    menu_args_exec(input_monitor_menus, sizeof(input_monitor_menus) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    //pr_win_gpio_depth--;
    return 0;
}

static void gpio_cpld_det_thread(void)
{
    int old, new;
    int x, y;
    int id = gpio_read(GPIO_PORT1, GPIO_PIN7);

    old = gpio_read(GPIO_PORT3, GPIO_PIN19);

    pr_win(pr_win_gpio[pr_win_gpio_depth], "ID[%d] = %s\n", id, old ? "active" : "inactive");

    while (1) {
        /* PLD_OUT read */
        new = gpio_read(GPIO_PORT3, GPIO_PIN19);
        if (new) {
            if (old != new) {
                /* PLD_OUT is changed */
                old = new;
                /* PLD_IN write */
                gpio_write(GPIO_PORT3, GPIO_PIN20, 0);
                gpio_write(GPIO_PORT2, GPIO_PIN0, 0);   // act led on
                pr_win(pr_win_gpio[pr_win_gpio_depth], "ID[%d] = %s\n", id, old ? "active" : "inactive");
            }
        } else {
            if (old != new) {
                /* PLD_OUT is changed */
                old = new;
                /* PLD_IN write */
                gpio_write(GPIO_PORT3, GPIO_PIN20, 1);
                gpio_write(GPIO_PORT2, GPIO_PIN0, 1);   // act led off
                pr_win(pr_win_gpio[pr_win_gpio_depth], "ID[%d] = %s\n", id, old ? "active" : "inactive");
            }
        }
        /* 100ms delay */
        usleep(100 * 1000);
    }
}

static void gpio_det_thread(void)
{
    int port, pin;
    int old, new;
    int x, y;
    int initflag = 1;

    while (1) {
        if (!gpio_input_monitoring) {
            initflag = 1;
            sleep(1);
            continue;
        } else {
            if (initflag) {
                sleep(1);
                initflag = 0;
            }
        }

        for (port = GPIO_PORT1; port < MAX_GPIO_PORT; port++) {
            for (pin = GPIO_PIN0; pin < MAX_GPIO_PIN; pin++) {
                if (!gpio_stat[port][pin].init_flag)
                    continue;

                if (port == GPIO_PORT1 &&
                    (pin != GPIO_PIN7 && pin != GPIO_PIN8 && pin != GPIO_PIN13 && pin != GPIO_PIN14))
                    continue;

                if (port == GPIO_PORT2 &&
                    (pin != GPIO_PIN0 && pin != GPIO_PIN1 && pin != GPIO_PIN2 && pin != GPIO_PIN3 && pin != GPIO_PIN4 &&
                        pin != GPIO_PIN5 && pin != GPIO_PIN6 && pin != GPIO_PIN7 && pin != GPIO_PIN8 &&
                        pin != GPIO_PIN9 && pin != GPIO_PIN10 && pin != GPIO_PIN20))
                    continue;

                if (port == GPIO_PORT3)
                    continue;

                if (port == GPIO_PORT4 &&
                    (pin != GPIO_PIN0 && pin != GPIO_PIN18 && pin != GPIO_PIN19 && pin != GPIO_PIN20))
                    continue;

                if (port == GPIO_PORT5)
                    continue;

                if (port == GPIO_PORT6 &&
                    (pin > GPIO_PIN7))
                    continue;

                if (port == GPIO_PORT7 &&
                    (pin > GPIO_PIN7))
                    continue;

                new = gpio_read(port, pin);
                if (new < 0) {
                    pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][INPUT]: error!\n", port + 1, pin);
                    continue;
                }

                old = gpio_stat[port][pin].inval;
                if (old != new) {
                    gpio_stat[port][pin].inval = new;
                    pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][INPUT]: \"%s\"\n", port + 1, pin, new ? "HIGH" : "LOW");
                }
                usleep(10 * 1000);
            }
        }
    }
}

static int gpio_read_read(void *val)
{
    int *v = (int *)val;
    int x, y, ret;

    ret = gpio_read(v[0], v[1]);
    if (ret < 0)
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][READ]: error!\n", v[0] + 1, v[1]);
    else
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][READ]: \"%s\"\n", v[0] + 1, v[1], ret ? "HIGH" : "LOW");

    return 0;
}

static int gpio_active_high(void *val)
{
    int *v = (int *)val;
    int x, y, ret;

    ret = gpio_write(v[0], v[1], 1);
    if (ret < 0)
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][WRITE]: error!\n", v[0] + 1, v[1]);
    else
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][WRITE]: \"%s\"\n", v[0] + 1, v[1], ret ? "HIGH" : "LOW");

    return 0;
}

static int gpio_active_low(void *val)
{
    int *v = (int *)val;
    int x, y, ret;

    ret = gpio_write(v[0], v[1], 0);
    if (ret < 0)
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][WRITE]: error!\n", v[0] + 1, v[1]);
    else
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][WRITE]: \"%s\"\n", v[0] + 1, v[1], ret ? "HIGH" : "LOW");

    return 0;
}

static int gpio_out_ctrl_pin0(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN0 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN0];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO00]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin1(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN1 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN1];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO01]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    // ret = gpio_write(*p, GPIO_PIN1, sig);
    // printf("[GPIO%d_IO01] : \"%s\"\n", *p, ret ? "HIGH" : "LOW");
    return 0;
}

static int gpio_out_ctrl_pin2(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN2 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN2];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val},
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO02]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin3(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN3 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN3];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO03]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin4(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN4 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN4];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO04]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin5(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN5 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN5];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO05]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin6(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN6 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN6];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO06]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin7(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN7 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN7];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO07]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin8(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN8 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN8];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO08]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin9(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN9 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN9];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO09]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin10(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN10 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN10];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO10]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin11(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN11 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN11];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO11]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin12(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN12 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN12];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO12]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin13(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN13 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN13];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO13]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin14(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN14 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN14];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO14]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin15(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN15 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN15];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO15]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin16(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN16 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN16];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO16]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin17(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN17 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN17];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO17]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin18(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN18 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN18];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO18]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin19(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN19 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN19];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO19]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin20(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN20 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN20];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO20]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin21(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN21 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN21];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO21]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin22(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN22 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN22];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO22]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin23(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN23 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN23];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO23]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin24(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN24 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN24];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO24]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin25(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN25 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN25];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO25]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin26(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN26 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN26];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO26]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin27(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN27 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN27];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO27]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin28(void *port)
{
    int *p = (int *)port;
    int y, x;
    int val[2] = { *p, GPIO_PIN28 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN28];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO28]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin29(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN29 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN29];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO29]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin30(void *port)
{
    int *p = (int *)port;
    int y, x;
    int val[2] = { *p, GPIO_PIN30 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN30];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO30]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}

static int gpio_out_ctrl_pin31(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN31 };
    char *des = "Select Signal(H/L)";
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN31];
    menu_args_t gpio_sig_menu[] = {
        {gpio_active_high, "HIGH", &val},
        {gpio_active_low, "LOW", &val},
        {back2, "back", &val}
    };

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO31]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    pr_win_gpio_depth++;
    menu_args_exec(gpio_sig_menu, sizeof(gpio_sig_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;

    return 0;
}



static int gpio_out_ctrl_port1(void)
{
    char *des = "GPIO PORT1 OUTPUT TEST MENU";
    int port = GPIO_PORT1;

    menu_args_t gpio_port1_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO1_IO00", &port},
        // {gpio_out_ctrl_pin1, "GPIO1_IO01", &port},
        // {gpio_out_ctrl_pin2, "GPIO1_IO02", &port},
        // {gpio_out_ctrl_pin3, "GPIO1_IO03", &port},
        // {gpio_out_ctrl_pin4, "GPIO1_IO04", &port},
        // {gpio_out_ctrl_pin5, "GPIO1_IO05", &port},
        // {gpio_out_ctrl_pin6, "GPIO1_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO1_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO1_IO08", &port},
        // {gpio_out_ctrl_pin9, "GPIO1_IO09", &port},
        // {gpio_out_ctrl_pin10, "GPIO1_IO10", &port},
        // {gpio_out_ctrl_pin11, "GPIO1_IO11", &port},
        // {gpio_out_ctrl_pin12, "GPIO1_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO1_IO13", &port},     // i2c-io rst pin, not used in gpio test
        // {gpio_out_ctrl_pin14, "GPIO1_IO14", &port},
        // {gpio_out_ctrl_pin15, "GPIO1_IO15", &port},
        // {gpio_out_ctrl_pin16, "GPIO1_IO16", &port},
        // {gpio_out_ctrl_pin17, "GPIO1_IO17", &port},
        // {gpio_out_ctrl_pin18, "GPIO1_IO18", &port},
        // {gpio_out_ctrl_pin19, "GPIO1_IO19", &port},
        // {gpio_out_ctrl_pin20, "GPIO1_IO20", &port},
        // {gpio_out_ctrl_pin21, "GPIO1_IO21", &port},
        // {gpio_out_ctrl_pin22, "GPIO1_IO22", &port},
        // {gpio_out_ctrl_pin23, "GPIO1_IO23", &port},
        // {gpio_out_ctrl_pin24, "GPIO1_IO24", &port},
        // {gpio_out_ctrl_pin25, "GPIO1_IO25", &port},
        // {gpio_out_ctrl_pin26, "GPIO1_IO26", &port},
        // {gpio_out_ctrl_pin27, "GPIO1_IO27", &port},
        // {gpio_out_ctrl_pin28, "GPIO1_IO28", &port},
        // {gpio_out_ctrl_pin29, "GPIO1_IO29", &port},
        // {gpio_out_ctrl_pin30, "GPIO1_IO30", &port},
        // {gpio_out_ctrl_pin31, "GPIO1_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port1_menu, sizeof(gpio_port1_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port2(void)
{
    char *des = "GPIO PORT2 OUTPUT TEST MENU";
    int port = GPIO_PORT2;

    menu_args_t gpio_port2_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO2_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO2_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO2_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO2_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO2_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO2_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO2_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO2_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO2_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO2_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO2_IO10", &port},
        // {gpio_out_ctrl_pin11, "GPIO2_IO11", &port},
        // {gpio_out_ctrl_pin12, "GPIO2_IO12", &port},
        // {gpio_out_ctrl_pin13, "GPIO2_IO13", &port},
        // {gpio_out_ctrl_pin14, "GPIO2_IO14", &port},
        // {gpio_out_ctrl_pin15, "GPIO2_IO15", &port},
        // {gpio_out_ctrl_pin16, "GPIO2_IO16", &port},
        // {gpio_out_ctrl_pin17, "GPIO2_IO17", &port},
        // {gpio_out_ctrl_pin18, "GPIO2_IO18", &port},
        // {gpio_out_ctrl_pin19, "GPIO2_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO2_IO20", &port},
        // {gpio_out_ctrl_pin21, "GPIO2_IO21", &port},
        // {gpio_out_ctrl_pin22, "GPIO2_IO22", &port},
        // {gpio_out_ctrl_pin23, "GPIO2_IO23", &port},
        // {gpio_out_ctrl_pin24, "GPIO2_IO24", &port},
        // {gpio_out_ctrl_pin25, "GPIO2_IO25", &port},
        // {gpio_out_ctrl_pin26, "GPIO2_IO26", &port},
        // {gpio_out_ctrl_pin27, "GPIO2_IO27", &port},
        // {gpio_out_ctrl_pin28, "GPIO2_IO28", &port},
        // {gpio_out_ctrl_pin29, "GPIO2_IO29", &port},
        // {gpio_out_ctrl_pin30, "GPIO2_IO30", &port},
        // {gpio_out_ctrl_pin31, "GPIO2_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port2_menu, sizeof(gpio_port2_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port3(void)
{
    char *des = "GPIO PORT3 OUTPUT TEST MENU";
    int port = GPIO_PORT3;

    menu_args_t gpio_port3_menu[] = {
        // {gpio_out_ctrl_pin0, "GPIO3_IO00", &port},
        // {gpio_out_ctrl_pin1, "GPIO3_IO01", &port},
        // {gpio_out_ctrl_pin2, "GPIO3_IO02", &port},
        // {gpio_out_ctrl_pin3, "GPIO3_IO03", &port},
        // {gpio_out_ctrl_pin4, "GPIO3_IO04", &port},
        // {gpio_out_ctrl_pin5, "GPIO3_IO05", &port},
        // {gpio_out_ctrl_pin6, "GPIO3_IO06", &port},
        // {gpio_out_ctrl_pin7, "GPIO3_IO07", &port},
        // {gpio_out_ctrl_pin8, "GPIO3_IO08", &port},
        // {gpio_out_ctrl_pin9, "GPIO3_IO09", &port},
        // {gpio_out_ctrl_pin10, "GPIO3_IO10", &port},
        // {gpio_out_ctrl_pin11, "GPIO3_IO11", &port},
        // {gpio_out_ctrl_pin12, "GPIO3_IO12", &port},
        // {gpio_out_ctrl_pin13, "GPIO3_IO13", &port},
        // {gpio_out_ctrl_pin14, "GPIO3_IO14", &port},
        // {gpio_out_ctrl_pin15, "GPIO3_IO15", &port},
        // {gpio_out_ctrl_pin16, "GPIO3_IO16", &port},
        // {gpio_out_ctrl_pin17, "GPIO3_IO17", &port},
        // {gpio_out_ctrl_pin18, "GPIO3_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO3_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO3_IO20", &port},
        {gpio_out_ctrl_pin21, "GPIO3_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO3_IO22", &port},
        // {gpio_out_ctrl_pin23, "GPIO3_IO23", &port},
        // {gpio_out_ctrl_pin24, "GPIO3_IO24", &port},
        // {gpio_out_ctrl_pin25, "GPIO3_IO25", &port},
        // {gpio_out_ctrl_pin26, "GPIO3_IO26", &port},
        // {gpio_out_ctrl_pin27, "GPIO3_IO27", &port},
        // {gpio_out_ctrl_pin28, "GPIO3_IO28", &port},
        // {gpio_out_ctrl_pin29, "GPIO3_IO29", &port},
        // {gpio_out_ctrl_pin30, "GPIO3_IO30", &port},
        // {gpio_out_ctrl_pin31, "GPIO3_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port3_menu, sizeof(gpio_port3_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port4(void)
{
    char *des = "GPIO PORT4 OUTPUT TEST MENU";
    int port = GPIO_PORT4;

    menu_args_t gpio_port4_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO4_IO00", &port},
        // {gpio_out_ctrl_pin1, "GPIO4_IO01", &port},
        // {gpio_out_ctrl_pin2, "GPIO4_IO02", &port},
        // {gpio_out_ctrl_pin3, "GPIO4_IO03", &port},
        // {gpio_out_ctrl_pin4, "GPIO4_IO04", &port},
        // {gpio_out_ctrl_pin5, "GPIO4_IO05", &port},
        // {gpio_out_ctrl_pin6, "GPIO4_IO06", &port},
        // {gpio_out_ctrl_pin7, "GPIO4_IO07", &port},
        // {gpio_out_ctrl_pin8, "GPIO4_IO08", &port},
        // {gpio_out_ctrl_pin9, "GPIO4_IO09", &port},
        // {gpio_out_ctrl_pin10, "GPIO4_IO10", &port},
        // {gpio_out_ctrl_pin11, "GPIO4_IO11", &port},
        // {gpio_out_ctrl_pin12, "GPIO4_IO12", &port},
        // {gpio_out_ctrl_pin13, "GPIO4_IO13", &port},
        // {gpio_out_ctrl_pin14, "GPIO4_IO14", &port},
        // {gpio_out_ctrl_pin15, "GPIO4_IO15", &port},
        // {gpio_out_ctrl_pin16, "GPIO4_IO16", &port},
        // {gpio_out_ctrl_pin17, "GPIO4_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO4_IO18", &port},
        // {gpio_out_ctrl_pin19, "GPIO4_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO4_IO20", &port},
        // {gpio_out_ctrl_pin21, "GPIO4_IO21", &port},
        // {gpio_out_ctrl_pin22, "GPIO4_IO22", &port},
        // {gpio_out_ctrl_pin23, "GPIO4_IO23", &port},
        // {gpio_out_ctrl_pin24, "GPIO4_IO24", &port},
        // {gpio_out_ctrl_pin25, "GPIO4_IO25", &port},
        // {gpio_out_ctrl_pin26, "GPIO4_IO26", &port},
        // {gpio_out_ctrl_pin27, "GPIO4_IO27", &port},
        // {gpio_out_ctrl_pin28, "GPIO4_IO28", &port},
        // {gpio_out_ctrl_pin29, "GPIO4_IO29", &port},
        // {gpio_out_ctrl_pin30, "GPIO4_IO30", &port},
        // {gpio_out_ctrl_pin31, "GPIO4_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port4_menu, sizeof(gpio_port4_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port5(void)
{
    char *des = "GPIO PORT5 OUTPUT TEST MENU";
    int port = GPIO_PORT5;

    menu_args_t gpio_port5_menu[] = {
        // {gpio_out_ctrl_pin0, "GPIO5_IO00", &port},
        // {gpio_out_ctrl_pin1, "GPIO5_IO01", &port},
        // {gpio_out_ctrl_pin2, "GPIO5_IO02", &port},
        // {gpio_out_ctrl_pin3, "GPIO5_IO03", &port},
        // {gpio_out_ctrl_pin4, "GPIO5_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO5_IO05", &port},
        // {gpio_out_ctrl_pin6, "GPIO5_IO06", &port},
        // {gpio_out_ctrl_pin7, "GPIO5_IO07", &port},
        // {gpio_out_ctrl_pin8, "GPIO5_IO08", &port},
        // {gpio_out_ctrl_pin9, "GPIO5_IO09", &port},
        // {gpio_out_ctrl_pin10, "GPIO5_IO10", &port},
        // {gpio_out_ctrl_pin11, "GPIO5_IO11", &port},
        // {gpio_out_ctrl_pin12, "GPIO5_IO12", &port},
        // {gpio_out_ctrl_pin13, "GPIO5_IO13", &port},
        // {gpio_out_ctrl_pin14, "GPIO5_IO14", &port},
        // {gpio_out_ctrl_pin15, "GPIO5_IO15", &port},
        // {gpio_out_ctrl_pin16, "GPIO5_IO16", &port},
        // {gpio_out_ctrl_pin17, "GPIO5_IO17", &port},
        // {gpio_out_ctrl_pin18, "GPIO5_IO18", &port},
        // {gpio_out_ctrl_pin19, "GPIO5_IO19", &port},
        // {gpio_out_ctrl_pin20, "GPIO5_IO20", &port},
        // {gpio_out_ctrl_pin21, "GPIO5_IO21", &port},
        // {gpio_out_ctrl_pin22, "GPIO5_IO22", &port},
        // {gpio_out_ctrl_pin23, "GPIO5_IO23", &port},
        // {gpio_out_ctrl_pin24, "GPIO5_IO24", &port},
        // {gpio_out_ctrl_pin25, "GPIO5_IO25", &port},
        // {gpio_out_ctrl_pin26, "GPIO5_IO26", &port},
        // {gpio_out_ctrl_pin27, "GPIO5_IO27", &port},
        // {gpio_out_ctrl_pin28, "GPIO5_IO28", &port},
        // {gpio_out_ctrl_pin29, "GPIO5_IO29", &port},
        // {gpio_out_ctrl_pin30, "GPIO5_IO30", &port},
        // {gpio_out_ctrl_pin31, "GPIO5_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port5_menu, sizeof(gpio_port5_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port6(void)
{
    char *des = "GPIO PORT6(I2C-IO) OUTPUT TEST MENU";
    int port = GPIO_PORT6;

    menu_args_t gpio_port6_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO6_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO6_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO6_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO6_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO6_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO6_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO6_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO6_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO6_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO6_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO6_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO6_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO6_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO6_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO6_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO6_IO15", &port},
#if 0
        {gpio_ctrl_pin16, "GPIO6_IO16", &port},
        {gpio_ctrl_pin17, "GPIO6_IO17", &port},
        {gpio_ctrl_pin18, "GPIO6_IO18", &port},
        {gpio_ctrl_pin19, "GPIO6_IO19", &port},
        {gpio_ctrl_pin20, "GPIO6_IO20", &port},
        {gpio_ctrl_pin21, "GPIO6_IO21", &port},
        {gpio_ctrl_pin22, "GPIO6_IO22", &port},
        {gpio_ctrl_pin23, "GPIO6_IO23", &port},
        {gpio_ctrl_pin24, "GPIO6_IO24", &port},
        {gpio_ctrl_pin25, "GPIO6_IO25", &port},
        {gpio_ctrl_pin26, "GPIO6_IO26", &port},
        {gpio_ctrl_pin27, "GPIO6_IO27", &port},
        {gpio_ctrl_pin28, "GPIO6_IO28", &port},
        {gpio_ctrl_pin29, "GPIO6_IO29", &port},
        {gpio_ctrl_pin30, "GPIO6_IO30", &port},
        {gpio_ctrl_pin31, "GPIO6_IO31", &port}
#endif
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port6_menu, sizeof(gpio_port6_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port7(void)
{
    char *des = "GPIO PORT7(SPI-IO) OUTPUT TEST MENU";
    int port = GPIO_PORT7;

    menu_args_t gpio_port7_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO7_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO7_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO7_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO7_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO7_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO7_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO7_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO7_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO7_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO7_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO7_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO7_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO7_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO7_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO7_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO7_IO15", &port},
#if 0
        {gpio_ctrl_pin16, "GPIO7_IO16", &port},
        {gpio_ctrl_pin17, "GPIO7_IO17", &port},
        {gpio_ctrl_pin18, "GPIO7_IO18", &port},
        {gpio_ctrl_pin19, "GPIO7_IO19", &port},
        {gpio_ctrl_pin20, "GPIO7_IO20", &port},
        {gpio_ctrl_pin21, "GPIO7_IO21", &port},
        {gpio_ctrl_pin22, "GPIO7_IO22", &port},
        {gpio_ctrl_pin23, "GPIO7_IO23", &port},
        {gpio_ctrl_pin24, "GPIO7_IO24", &port},
        {gpio_ctrl_pin25, "GPIO7_IO25", &port},
        {gpio_ctrl_pin26, "GPIO7_IO26", &port},
        {gpio_ctrl_pin27, "GPIO7_IO27", &port},
        {gpio_ctrl_pin28, "GPIO7_IO28", &port},
        {gpio_ctrl_pin29, "GPIO7_IO29", &port},
        {gpio_ctrl_pin30, "GPIO7_IO30", &port},
        {gpio_ctrl_pin31, "GPIO7_IO31", &port}
#endif
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port7_menu, sizeof(gpio_port7_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_pin0(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN0 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN0];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO00]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);

    return 0;
}

static int gpio_in_ctrl_pin1(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN1 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN1];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO01]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin2(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN2 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN2];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO02]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin3(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN3 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN3];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO03]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin4(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN4 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN4];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO04]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin5(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN5 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN5];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO05]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin6(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN6 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN6];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO06]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin7(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN7 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN7];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO07]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin8(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN8 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN8];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO08]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin9(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN9 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN9];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO09]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin10(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN10 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN10];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO10]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin11(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN11 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN11];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO11]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin12(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN12 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN12];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO12]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin13(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN13 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN13];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO13]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin14(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN14 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN14];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO14]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin15(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN15 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN15];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO15]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin16(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN16 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN16];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO16]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin17(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN17 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN17];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO17]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }

    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin18(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN18 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN18];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO18]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin19(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN19 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN19];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO19]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin20(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN20 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN20];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO20]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin21(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN21 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN21];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO21]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin22(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN22 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN22];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO22]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin23(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN23 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN23];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO23]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin24(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN24 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN24];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO24]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin25(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN25 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN25];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO25]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin26(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN26 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN26];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO26]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin27(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN27 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN27];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO27]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin28(void *port)
{
    int *p = (int *)port;
    int y, x;
    int val[2] = { *p, GPIO_PIN28 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN28];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO28]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin29(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN29 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN29];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO29]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin30(void *port)
{
    int *p = (int *)port;
    int y, x;
    int val[2] = { *p, GPIO_PIN30 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN30];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO30]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}

static int gpio_in_ctrl_pin31(void *port)
{
    int *p = (int *)port;
    int x, y;
    int val[2] = { *p, GPIO_PIN31 };
    gpiostat_t *stat = &gpio_stat[*p][GPIO_PIN31];

    if (!stat->init_flag) {
        pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO31]: already pin used! can't control this pin.\n", *p + 1);
        return 0;
    }
    gpio_read_read(val);
    return 0;
}


static int gpio_in_ctrl_port1(void)
{
    char *des = "GPIO PORT1 INPUT TEST MENU";
    int port = GPIO_PORT1;

    menu_args_t gpio_port1_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO1_IO00", &port},
        // {gpio_in_ctrl_pin1, "GPIO1_IO01", &port},
        // {gpio_in_ctrl_pin2, "GPIO1_IO02", &port},
        // {gpio_in_ctrl_pin3, "GPIO1_IO03", &port},
        // {gpio_in_ctrl_pin4, "GPIO1_IO04", &port},
        // {gpio_in_ctrl_pin5, "GPIO1_IO05", &port},
        // {gpio_in_ctrl_pin6, "GPIO1_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO1_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO1_IO08", &port},
        // {gpio_in_ctrl_pin9, "GPIO1_IO09", &port},
        // {gpio_in_ctrl_pin10, "GPIO1_IO10", &port},
        // {gpio_in_ctrl_pin11, "GPIO1_IO11", &port},
        // {gpio_in_ctrl_pin12, "GPIO1_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO1_IO13", &port},
        // {gpio_in_ctrl_pin14, "GPIO1_IO14", &port},       // i2c-io rst pin
        // {gpio_in_ctrl_pin15, "GPIO1_IO15", &port},
        // {gpio_in_ctrl_pin16, "GPIO1_IO16", &port},
        // {gpio_in_ctrl_pin17, "GPIO1_IO17", &port},
        // {gpio_in_ctrl_pin18, "GPIO1_IO18", &port},
        // {gpio_in_ctrl_pin19, "GPIO1_IO19", &port},
        // {gpio_in_ctrl_pin20, "GPIO1_IO20", &port},
        // {gpio_in_ctrl_pin21, "GPIO1_IO21", &port},
        // {gpio_in_ctrl_pin22, "GPIO1_IO22", &port},
        // {gpio_in_ctrl_pin23, "GPIO1_IO23", &port},
        // {gpio_in_ctrl_pin24, "GPIO1_IO24", &port},
        // {gpio_in_ctrl_pin25, "GPIO1_IO25", &port},
        // {gpio_in_ctrl_pin26, "GPIO1_IO26", &port},
        // {gpio_in_ctrl_pin27, "GPIO1_IO27", &port},
        // {gpio_in_ctrl_pin28, "GPIO1_IO28", &port},
        // {gpio_in_ctrl_pin29, "GPIO1_IO29", &port},
        // {gpio_in_ctrl_pin30, "GPIO1_IO30", &port},
        // {gpio_in_ctrl_pin31, "GPIO1_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port1_menu, sizeof(gpio_port1_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port2(void)
{
    char *des = "GPIO PORT2 INPUT TEST MENU";
    int port = GPIO_PORT2;

    menu_args_t gpio_port2_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO2_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO2_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO2_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO2_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO2_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO2_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO2_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO2_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO2_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO2_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO2_IO10", &port},
        // {gpio_in_ctrl_pin11, "GPIO2_IO11", &port},
        // {gpio_in_ctrl_pin12, "GPIO2_IO12", &port},
        // {gpio_in_ctrl_pin13, "GPIO2_IO13", &port},
        // {gpio_in_ctrl_pin14, "GPIO2_IO14", &port},
        // {gpio_in_ctrl_pin15, "GPIO2_IO15", &port},
        // {gpio_in_ctrl_pin16, "GPIO2_IO16", &port},
        // {gpio_in_ctrl_pin17, "GPIO2_IO17", &port},
        // {gpio_in_ctrl_pin18, "GPIO2_IO18", &port},
        // {gpio_in_ctrl_pin19, "GPIO2_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO2_IO20", &port},
        // {gpio_in_ctrl_pin21, "GPIO2_IO21", &port},
        // {gpio_in_ctrl_pin22, "GPIO2_IO22", &port},
        // {gpio_in_ctrl_pin23, "GPIO2_IO23", &port},
        // {gpio_in_ctrl_pin24, "GPIO2_IO24", &port},
        // {gpio_in_ctrl_pin25, "GPIO2_IO25", &port},
        // {gpio_in_ctrl_pin26, "GPIO2_IO26", &port},
        // {gpio_in_ctrl_pin27, "GPIO2_IO27", &port},
        // {gpio_in_ctrl_pin28, "GPIO2_IO28", &port},
        // {gpio_in_ctrl_pin29, "GPIO2_IO29", &port},
        // {gpio_in_ctrl_pin30, "GPIO2_IO30", &port},
        // {gpio_in_ctrl_pin31, "GPIO2_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port2_menu, sizeof(gpio_port2_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port3(void)
{
    char *des = "GPIO PORT3 INPUT TEST MENU";
    int port = GPIO_PORT3;

    menu_args_t gpio_port3_menu[] = {
        // {gpio_in_ctrl_pin0, "GPIO3_IO00", &port},
        // {gpio_in_ctrl_pin1, "GPIO3_IO01", &port},
        // {gpio_in_ctrl_pin2, "GPIO3_IO02", &port},
        // {gpio_in_ctrl_pin3, "GPIO3_IO03", &port},
        // {gpio_in_ctrl_pin4, "GPIO3_IO04", &port},
        // {gpio_in_ctrl_pin5, "GPIO3_IO05", &port},
        // {gpio_in_ctrl_pin6, "GPIO3_IO06", &port},
        // {gpio_in_ctrl_pin7, "GPIO3_IO07", &port},
        // {gpio_in_ctrl_pin8, "GPIO3_IO08", &port},
        // {gpio_in_ctrl_pin9, "GPIO3_IO09", &port},
        // {gpio_in_ctrl_pin10, "GPIO3_IO10", &port},
        // {gpio_in_ctrl_pin11, "GPIO3_IO11", &port},
        // {gpio_in_ctrl_pin12, "GPIO3_IO12", &port},
        // {gpio_in_ctrl_pin13, "GPIO3_IO13", &port},
        // {gpio_in_ctrl_pin14, "GPIO3_IO14", &port},
        // {gpio_in_ctrl_pin15, "GPIO3_IO15", &port},
        // {gpio_in_ctrl_pin16, "GPIO3_IO16", &port},
        // {gpio_in_ctrl_pin17, "GPIO3_IO17", &port},
        // {gpio_in_ctrl_pin18, "GPIO3_IO18", &port},
        // {gpio_in_ctrl_pin19, "GPIO3_IO19", &port},
        // {gpio_in_ctrl_pin20, "GPIO3_IO20", &port},
        {gpio_in_ctrl_pin21, "GPIO3_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO3_IO22", &port},
        // {gpio_in_ctrl_pin23, "GPIO3_IO23", &port},
        // {gpio_in_ctrl_pin24, "GPIO3_IO24", &port},
        // {gpio_in_ctrl_pin25, "GPIO3_IO25", &port},
        // {gpio_in_ctrl_pin26, "GPIO3_IO26", &port},
        // {gpio_in_ctrl_pin27, "GPIO3_IO27", &port},
        // {gpio_in_ctrl_pin28, "GPIO3_IO28", &port},
        // {gpio_in_ctrl_pin29, "GPIO3_IO29", &port},
        // {gpio_in_ctrl_pin30, "GPIO3_IO30", &port},
        // {gpio_in_ctrl_pin31, "GPIO3_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port3_menu, sizeof(gpio_port3_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port4(void)
{
    char *des = "GPIO PORT4 INPUT TEST MENU";
    int port = GPIO_PORT4;

    menu_args_t gpio_port4_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO4_IO00", &port},
        // {gpio_in_ctrl_pin1, "GPIO4_IO01", &port},
        // {gpio_in_ctrl_pin2, "GPIO4_IO02", &port},
        // {gpio_in_ctrl_pin3, "GPIO4_IO03", &port},
        // {gpio_in_ctrl_pin4, "GPIO4_IO04", &port},
        // {gpio_in_ctrl_pin5, "GPIO4_IO05", &port},
        // {gpio_in_ctrl_pin6, "GPIO4_IO06", &port},
        // {gpio_in_ctrl_pin7, "GPIO4_IO07", &port},
        // {gpio_in_ctrl_pin8, "GPIO4_IO08", &port},
        // {gpio_in_ctrl_pin9, "GPIO4_IO09", &port},
        // {gpio_in_ctrl_pin10, "GPIO4_IO10", &port},
        // {gpio_in_ctrl_pin11, "GPIO4_IO11", &port},
        // {gpio_in_ctrl_pin12, "GPIO4_IO12", &port},
        // {gpio_in_ctrl_pin13, "GPIO4_IO13", &port},
        // {gpio_in_ctrl_pin14, "GPIO4_IO14", &port},
        // {gpio_in_ctrl_pin15, "GPIO4_IO15", &port},
        // {gpio_in_ctrl_pin16, "GPIO4_IO16", &port},
        // {gpio_in_ctrl_pin17, "GPIO4_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO4_IO18", &port},
        // {gpio_in_ctrl_pin19, "GPIO4_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO4_IO20", &port},
        // {gpio_in_ctrl_pin21, "GPIO4_IO21", &port},
        // {gpio_in_ctrl_pin22, "GPIO4_IO22", &port},
        // {gpio_in_ctrl_pin23, "GPIO4_IO23", &port},
        // {gpio_in_ctrl_pin24, "GPIO4_IO24", &port},
        // {gpio_in_ctrl_pin25, "GPIO4_IO25", &port},
        // {gpio_in_ctrl_pin26, "GPIO4_IO26", &port},
        // {gpio_in_ctrl_pin27, "GPIO4_IO27", &port},
        // {gpio_in_ctrl_pin28, "GPIO4_IO28", &port},
        // {gpio_in_ctrl_pin29, "GPIO4_IO29", &port},
        // {gpio_in_ctrl_pin30, "GPIO4_IO30", &port},
        // {gpio_in_ctrl_pin31, "GPIO4_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port4_menu, sizeof(gpio_port4_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port5(void)
{
    char *des = "GPIO PORT5 INPUT TEST MENU";
    int port = GPIO_PORT5;

    menu_args_t gpio_port5_menu[] = {
        // {gpio_in_ctrl_pin0, "GPIO5_IO00", &port},
        // {gpio_in_ctrl_pin1, "GPIO5_IO01", &port},
        // {gpio_in_ctrl_pin2, "GPIO5_IO02", &port},
        // {gpio_in_ctrl_pin3, "GPIO5_IO03", &port},
        // {gpio_in_ctrl_pin4, "GPIO5_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO5_IO05", &port},
        // {gpio_in_ctrl_pin6, "GPIO5_IO06", &port},
        // {gpio_in_ctrl_pin7, "GPIO5_IO07", &port},
        // {gpio_in_ctrl_pin8, "GPIO5_IO08", &port},
        // {gpio_in_ctrl_pin9, "GPIO5_IO09", &port},
        // {gpio_in_ctrl_pin10, "GPIO5_IO10", &port},
        // {gpio_in_ctrl_pin11, "GPIO5_IO11", &port},
        // {gpio_in_ctrl_pin12, "GPIO5_IO12", &port},
        // {gpio_in_ctrl_pin13, "GPIO5_IO13", &port},
        // {gpio_in_ctrl_pin14, "GPIO5_IO14", &port},
        // {gpio_in_ctrl_pin15, "GPIO5_IO15", &port},
        // {gpio_in_ctrl_pin16, "GPIO5_IO16", &port},
        // {gpio_in_ctrl_pin17, "GPIO5_IO17", &port},
        // {gpio_in_ctrl_pin18, "GPIO5_IO18", &port},
        // {gpio_in_ctrl_pin19, "GPIO5_IO19", &port},
        // {gpio_in_ctrl_pin20, "GPIO5_IO20", &port},
        // {gpio_in_ctrl_pin21, "GPIO5_IO21", &port},
        // {gpio_in_ctrl_pin22, "GPIO5_IO22", &port},
        // {gpio_in_ctrl_pin23, "GPIO5_IO23", &port},
        // {gpio_in_ctrl_pin24, "GPIO5_IO24", &port},
        // {gpio_in_ctrl_pin25, "GPIO5_IO25", &port},
        // {gpio_in_ctrl_pin26, "GPIO5_IO26", &port},
        // {gpio_in_ctrl_pin27, "GPIO5_IO27", &port},
        // {gpio_in_ctrl_pin28, "GPIO5_IO28", &port},
        // {gpio_in_ctrl_pin29, "GPIO5_IO29", &port},
        // {gpio_in_ctrl_pin30, "GPIO5_IO30", &port},
        // {gpio_in_ctrl_pin31, "GPIO5_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port5_menu, sizeof(gpio_port5_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port6(void)
{
    char *des = "GPIO PORT6(I2C-IO) INPUT TEST MENU";
    int port = GPIO_PORT6;

    menu_args_t gpio_port6_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO6_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO6_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO6_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO6_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO6_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO6_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO6_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO6_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO6_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO6_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO6_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO6_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO6_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO6_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO6_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO6_IO15", &port},
#if 0
        {gpio_ctrl_pin16, "GPIO6_IO16", &port},
        {gpio_ctrl_pin17, "GPIO6_IO17", &port},
        {gpio_ctrl_pin18, "GPIO6_IO18", &port},
        {gpio_ctrl_pin19, "GPIO6_IO19", &port},
        {gpio_ctrl_pin20, "GPIO6_IO20", &port},
        {gpio_ctrl_pin21, "GPIO6_IO21", &port},
        {gpio_ctrl_pin22, "GPIO6_IO22", &port},
        {gpio_ctrl_pin23, "GPIO6_IO23", &port},
        {gpio_ctrl_pin24, "GPIO6_IO24", &port},
        {gpio_ctrl_pin25, "GPIO6_IO25", &port},
        {gpio_ctrl_pin26, "GPIO6_IO26", &port},
        {gpio_ctrl_pin27, "GPIO6_IO27", &port},
        {gpio_ctrl_pin28, "GPIO6_IO28", &port},
        {gpio_ctrl_pin29, "GPIO6_IO29", &port},
        {gpio_ctrl_pin30, "GPIO6_IO30", &port},
        {gpio_ctrl_pin31, "GPIO6_IO31", &port}
#endif
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port6_menu, sizeof(gpio_port6_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port7(void)
{
    char *des = "GPIO PORT7(SPI-IO) INPUT TEST MENU";
    int port = GPIO_PORT7;

    menu_args_t gpio_port7_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO7_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO7_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO7_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO7_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO7_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO7_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO7_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO7_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO7_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO7_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO7_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO7_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO7_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO7_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO7_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO7_IO15", &port},
#if 0
        {gpio_ctrl_pin16, "GPIO7_IO16", &port},
        {gpio_ctrl_pin17, "GPIO7_IO17", &port},
        {gpio_ctrl_pin18, "GPIO7_IO18", &port},
        {gpio_ctrl_pin19, "GPIO7_IO19", &port},
        {gpio_ctrl_pin20, "GPIO7_IO20", &port},
        {gpio_ctrl_pin21, "GPIO7_IO21", &port},
        {gpio_ctrl_pin22, "GPIO7_IO22", &port},
        {gpio_ctrl_pin23, "GPIO7_IO23", &port},
        {gpio_ctrl_pin24, "GPIO7_IO24", &port},
        {gpio_ctrl_pin25, "GPIO7_IO25", &port},
        {gpio_ctrl_pin26, "GPIO7_IO26", &port},
        {gpio_ctrl_pin27, "GPIO7_IO27", &port},
        {gpio_ctrl_pin28, "GPIO7_IO28", &port},
        {gpio_ctrl_pin29, "GPIO7_IO29", &port},
        {gpio_ctrl_pin30, "GPIO7_IO30", &port},
        {gpio_ctrl_pin31, "GPIO7_IO31", &port}
#endif
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port7_menu, sizeof(gpio_port7_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio1_io07_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT1, GPIO_PIN7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO07][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO07][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_1_7 = 1;
        sig_1_7 = !sig_1_7;
        ret = gpio_write(GPIO_PORT1, GPIO_PIN7, sig_1_7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO07][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO07][OUTPUT]: \"%s\"\n", sig_1_7 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio1_io08_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT1, GPIO_PIN8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO08][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO08][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_1_8 = 1;
        sig_1_8 = !sig_1_8;
        ret = gpio_write(GPIO_PORT1, GPIO_PIN8, sig_1_8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO08][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO08][OUTPUT]: \"%s\"\n", sig_1_8 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio1_io13_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT1, GPIO_PIN13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO13][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO13][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_1_13 = 1;
        sig_1_13 = !sig_1_13;
        ret = gpio_write(GPIO_PORT1, GPIO_PIN13, sig_1_13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO13][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO13][OUTPUT]: \"%s\"\n", sig_1_13 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio1_io14_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT1, GPIO_PIN14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO14][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO14][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_1_14 = 1;
        sig_1_14 = !sig_1_14;
        ret = gpio_write(GPIO_PORT1, GPIO_PIN14, sig_1_14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO14][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO1_IO14][OUTPUT]: \"%s\"\n", sig_1_14 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io00_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO00][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO00][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_0 = 1;
        sig_2_0 = !sig_2_0;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN0, sig_2_0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO00][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO00][OUTPUT]: \"%s\"\n", sig_2_0 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io01_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO01][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO01][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_1 = 1;
        sig_2_1 = !sig_2_1;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN1, sig_2_1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO01][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO01][OUTPUT]: \"%s\"\n", sig_2_1 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io02_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO02][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO02][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_2 = 1;
        sig_2_2 = !sig_2_2;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN2, sig_2_2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO02][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO02][OUTPUT]: \"%s\"\n", sig_2_2 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io03_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO03][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO03][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_3 = 1;
        sig_2_3 = !sig_2_3;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN3, sig_2_3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO03][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO03][OUTPUT]: \"%s\"\n", sig_2_3 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io04_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO04][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO04][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_4 = 1;
        sig_2_4 = !sig_2_4;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN4, sig_2_4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO04][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO04][OUTPUT]: \"%s\"\n", sig_2_4 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io05_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO05][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO05][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_5 = 1;
        sig_2_5 = !sig_2_5;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN5, sig_2_5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO05][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO05][OUTPUT]: \"%s\"\n", sig_2_5 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io06_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO06][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO06][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_6 = 1;
        sig_2_6 = !sig_2_6;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN6, sig_2_6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO06][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO06][OUTPUT]: \"%s\"\n", sig_2_6 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io07_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO07][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO07][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_7 = 1;
        sig_2_7 = !sig_2_7;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN7, sig_2_7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO07][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO07][OUTPUT]: \"%s\"\n", sig_2_7 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io08_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO08][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO08][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_8 = 1;
        sig_2_8 = !sig_2_8;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN8, sig_2_8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO08][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO08][OUTPUT]: \"%s\"\n", sig_2_8 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io09_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO09][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO09][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_9 = 1;
        sig_2_9 = !sig_2_9;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN9, sig_2_9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO09][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO09][OUTPUT]: \"%s\"\n", sig_2_9 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io10_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO10][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO10][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_10 = 1;
        sig_2_10 = !sig_2_10;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN10, sig_2_10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO10][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO10][OUTPUT]: \"%s\"\n", sig_2_10 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio2_io20_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT2, GPIO_PIN20);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO20][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO20][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_2_20 = 1;
        sig_2_20 = !sig_2_20;
        ret = gpio_write(GPIO_PORT2, GPIO_PIN20, sig_2_20);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO20][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO2_IO20][OUTPUT]: \"%s\"\n", sig_2_20 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio4_io00_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT4, GPIO_PIN0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO00][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO00][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_4_0 = 1;
        sig_4_0 = !sig_4_0;
        ret = gpio_write(GPIO_PORT4, GPIO_PIN0, sig_4_0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO00][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO00][OUTPUT]: \"%s\"\n", sig_4_0 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio4_io18_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT4, GPIO_PIN18);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO18][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO18][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_4_18 = 1;
        sig_4_18 = !sig_4_18;
        ret = gpio_write(GPIO_PORT4, GPIO_PIN18, sig_4_18);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO18][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO18][OUTPUT]: \"%s\"\n", sig_4_18 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio4_io19_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT4, GPIO_PIN19);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO19][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO19][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_4_19 = 1;
        sig_4_19 = !sig_4_19;
        ret = gpio_write(GPIO_PORT4, GPIO_PIN19, sig_4_19);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO19][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO19][OUTPUT]: \"%s\"\n", sig_4_19 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio4_io20_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT4, GPIO_PIN20);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO20][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO20][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_4_20 = 1;
        sig_4_20 = !sig_4_20;
        ret = gpio_write(GPIO_PORT4, GPIO_PIN20, sig_4_20);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO20][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO4_IO20][OUTPUT]: \"%s\"\n", sig_4_20 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io00_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO00][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO00][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_0 = 1;
        sig_6_0 = !sig_6_0;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN0, sig_6_0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO00][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO00][OUTPUT]: \"%s\"\n", sig_6_0 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io01_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO01][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO01][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_1 = 1;
        sig_6_1 = !sig_6_1;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN1, sig_6_1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO01][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO01][OUTPUT]: \"%s\"\n", sig_6_1 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io02_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO02][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO02][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_2 = 1;
        sig_6_2 = !sig_6_2;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN2, sig_6_2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO02][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO02][OUTPUT]: \"%s\"\n", sig_6_2 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io03_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO03][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO03][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_3 = 1;
        sig_6_3 = !sig_6_3;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN3, sig_6_3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO03][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO03][OUTPUT]: \"%s\"\n", sig_6_3 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io04_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO04][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO04][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_4 = 1;
        sig_6_4 = !sig_6_4;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN4, sig_6_4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO04][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO04][OUTPUT]: \"%s\"\n", sig_6_4 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io05_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO05][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO05][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_5 = 1;
        sig_6_5 = !sig_6_5;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN5, sig_6_5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO05][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO05][OUTPUT]: \"%s\"\n", sig_6_5 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io06_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO06][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO06][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_6 = 1;
        sig_6_6 = !sig_6_6;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN6, sig_6_6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO06][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO06][OUTPUT]: \"%s\"\n", sig_6_6 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io07_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO07][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO07][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_7 = 1;
        sig_6_7 = !sig_6_7;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN7, sig_6_7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO07][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO07][OUTPUT]: \"%s\"\n", sig_6_7 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io08_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO08][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO08][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_8 = 1;
        sig_6_8 = !sig_6_8;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN8, sig_6_8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO08][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO08][OUTPUT]: \"%s\"\n", sig_6_8 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io09_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO09][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO09][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_9 = 1;
        sig_6_9 = !sig_6_9;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN9, sig_6_9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO09][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO09][OUTPUT]: \"%s\"\n", sig_6_9 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io10_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO10][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO10][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_10 = 1;
        sig_6_10 = !sig_6_10;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN10, sig_6_10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO10][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO10][OUTPUT]: \"%s\"\n", sig_6_10 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io11_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN11);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO11][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO11][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_11 = 1;
        sig_6_11 = !sig_6_11;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN11, sig_6_11);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO11][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO11][OUTPUT]: \"%s\"\n", sig_6_11 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io12_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN12);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO12][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO12][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_12 = 1;
        sig_6_12 = !sig_6_12;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN12, sig_6_12);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO12][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO12][OUTPUT]: \"%s\"\n", sig_6_12 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io13_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO13][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO13][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_13 = 1;
        sig_6_13 = !sig_6_13;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN13, sig_6_13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO13][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO13][OUTPUT]: \"%s\"\n", sig_6_13 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io14_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO14][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO14][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_14 = 1;
        sig_6_14 = !sig_6_14;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN14, sig_6_14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO14][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO14][OUTPUT]: \"%s\"\n", sig_6_14 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio6_io15_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT6, GPIO_PIN15);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO15][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO15][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_6_15 = 1;
        sig_6_15 = !sig_6_15;
        ret = gpio_write(GPIO_PORT6, GPIO_PIN15, sig_6_15);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO15][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO6_IO15][OUTPUT]: \"%s\"\n", sig_6_15 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io00_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO00][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO00][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_0 = 1;
        sig_7_0 = !sig_7_0;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN0, sig_7_0);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO00][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO00][OUTPUT]: \"%s\"\n", sig_7_0 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io01_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO01][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO01][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_1 = 1;
        sig_7_1 = !sig_7_1;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN1, sig_7_1);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO01][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO01][OUTPUT]: \"%s\"\n", sig_7_1 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io02_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO02][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO02][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_2 = 1;
        sig_7_2 = !sig_7_2;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN2, sig_7_2);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO02][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO02][OUTPUT]: \"%s\"\n", sig_7_2 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io03_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO03][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO03][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_3 = 1;
        sig_7_3 = !sig_7_3;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN3, sig_7_3);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO03][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO03][OUTPUT]: \"%s\"\n", sig_7_3 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io04_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO04][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO04][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_4 = 1;
        sig_7_4 = !sig_7_4;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN4, sig_7_4);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO04][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO04][OUTPUT]: \"%s\"\n", sig_7_4 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io05_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO05][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO05][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_5 = 1;
        sig_7_5 = !sig_7_5;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN5, sig_7_5);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO05][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO05][OUTPUT]: \"%s\"\n", sig_7_5 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io06_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO06][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO06][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_6 = 1;
        sig_7_6 = !sig_7_6;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN6, sig_7_6);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO06][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO06][OUTPUT]: \"%s\"\n", sig_7_6 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io07_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO07][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO07][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_7 = 1;
        sig_7_7 = !sig_7_7;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN7, sig_7_7);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO07][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO07][OUTPUT]: \"%s\"\n", sig_7_7 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io08_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO08][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO08][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_8 = 1;
        sig_7_8 = !sig_7_8;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN8, sig_7_8);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO08][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO08][OUTPUT]: \"%s\"\n", sig_7_8 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io09_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO09][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO09][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_9 = 1;
        sig_7_9 = !sig_7_9;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN9, sig_7_9);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO09][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO09][OUTPUT]: \"%s\"\n", sig_7_9 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io10_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO10][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO10][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_10 = 1;
        sig_7_10 = !sig_7_10;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN10, sig_7_10);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO10][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO10][OUTPUT]: \"%s\"\n", sig_7_10 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io11_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN11);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO11][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO11][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_11 = 1;
        sig_7_11 = !sig_7_11;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN11, sig_7_11);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO11][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO11][OUTPUT]: \"%s\"\n", sig_7_11 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io12_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN12);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO12][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO12][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_12 = 1;
        sig_7_12 = !sig_7_12;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN12, sig_7_12);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO12][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO12][OUTPUT]: \"%s\"\n", sig_7_12 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io13_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO13][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO13][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_13 = 1;
        sig_7_13 = !sig_7_13;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN13, sig_7_13);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO13][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO13][OUTPUT]: \"%s\"\n", sig_7_13 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io14_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO14][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO14][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_14 = 1;
        sig_7_14 = !sig_7_14;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN14, sig_7_14);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO14][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO14][OUTPUT]: \"%s\"\n", sig_7_14 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int gpio7_io15_rw(void *rwflag)
{
    int *flag = (int *)rwflag;
    int ret;

    if (*flag) {
        ret = gpio_read(GPIO_PORT7, GPIO_PIN15);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO15][INPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO15][INPUT]: \"%s\"\n", ret ? "HIGH" : "LOW");;
        }
    } else {
        static int sig_7_15 = 1;
        sig_7_15 = !sig_7_15;
        ret = gpio_write(GPIO_PORT7, GPIO_PIN15, sig_7_15);
        if (ret < 0) {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO15][OUTPUT]: error!\n");
        } else {
            pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO7_IO15][OUTPUT]: \"%s\"\n", sig_7_15 ? "HIGH" : "LOW");
        }
    }
    return 0;
}

static int back_on_gpio(void *arg)
{
    gpio_input_monitoring = 0;
    return -1;
}

static int rflag = 1;
static menu_args_t gpio_in_menu[] = {
    //{gpio_input_monitor, "GPIO MONITORING", NULL},
    {gpio1_io07_rw, "GPIO1_IO07 (IO1_07)", &rflag},
    {gpio1_io08_rw, "GPIO1_IO08 (IO1_08)", &rflag},
    {gpio1_io13_rw, "GPIO1_IO13 (IO1_13)", &rflag},
    {gpio1_io14_rw, "GPIO1_IO14 (IO1_14)", &rflag},
    {gpio2_io00_rw, "GPIO2_IO00 (IO2_00)", &rflag},
    {gpio2_io01_rw, "GPIO2_IO01 (IO2_01)", &rflag},
    {gpio2_io02_rw, "GPIO2_IO02 (IO2_02)", &rflag},
    {gpio2_io03_rw, "GPIO2_IO03 (IO2_03)", &rflag},
    {gpio2_io04_rw, "GPIO2_IO04 (IO2_04)", &rflag},
    {gpio2_io05_rw, "GPIO2_IO05 (IO2_05)", &rflag},
    {gpio2_io06_rw, "GPIO2_IO06 (IO2_06)", &rflag},
    {gpio2_io07_rw, "GPIO2_IO07 (IO2_07)", &rflag},
    {gpio2_io08_rw, "GPIO2_IO08 (IO2_08)", &rflag},
    {gpio2_io09_rw, "GPIO2_IO09 (IO2_09)", &rflag},
    {gpio2_io10_rw, "GPIO2_IO10 (IO2_10)", &rflag},
    {gpio2_io20_rw, "GPIO2_IO20 (IO2_20)", &rflag},
    {gpio4_io00_rw, "GPIO4_IO00 (IO4_00)", &rflag},
    {gpio4_io18_rw, "GPIO4_IO18 (IO4_18)", &rflag},
    {gpio4_io19_rw, "GPIO4_IO19 (IO4_19)", &rflag},
    {gpio4_io20_rw, "GPIO4_IO20 (IO4_20)", &rflag},
    {gpio6_io00_rw, "GPIO6_IO00 (I2C_I0)", &rflag},
    {gpio6_io01_rw, "GPIO6_IO01 (I2C_I1)", &rflag},
    {gpio6_io02_rw, "GPIO6_IO02 (I2C_I2)", &rflag},
    {gpio6_io03_rw, "GPIO6_IO03 (I2C_I3)", &rflag},
    {gpio6_io04_rw, "GPIO6_IO04 (I2C_I4)", &rflag},
    {gpio6_io05_rw, "GPIO6_IO05 (I2C_I5)", &rflag},
    {gpio6_io06_rw, "GPIO6_IO06 (I2C_I6)", &rflag},
    {gpio6_io07_rw, "GPIO6_IO07 (I2C_I7)", &rflag},
    {gpio7_io00_rw, "GPIO7_IO00 (SPI_I0)", &rflag},
    {gpio7_io01_rw, "GPIO7_IO01 (SPI_I1)", &rflag},
    {gpio7_io02_rw, "GPIO7_IO02 (SPI_I2)", &rflag},
    {gpio7_io03_rw, "GPIO7_IO03 (SPI_I3)", &rflag},
    {gpio7_io04_rw, "GPIO7_IO04 (SPI_I4)", &rflag},
    {gpio7_io05_rw, "GPIO7_IO05 (SPI_I5)", &rflag},
    {gpio7_io06_rw, "GPIO7_IO06 (SPI_I6)", &rflag},
    {gpio7_io07_rw, "GPIO7_IO07 (SPI_I7)", &rflag},
    {back_on_gpio, "back", NULL},
};

static int wflag = 0;
static menu_args_t gpio_out_menu[] = {
    {gpio1_io07_rw, "GPIO1_IO07 (IO1_07)", &wflag},
    {gpio1_io08_rw, "GPIO1_IO08 (IO1_08)", &wflag},
    {gpio1_io13_rw, "GPIO1_IO13 (IO1_13)", &wflag},
    {gpio1_io14_rw, "GPIO1_IO14 (IO1_14)", &wflag},
    {gpio2_io00_rw, "GPIO2_IO00 (IO2_00)", &wflag},
    {gpio2_io01_rw, "GPIO2_IO01 (IO2_01)", &wflag},
    {gpio2_io02_rw, "GPIO2_IO02 (IO2_02)", &wflag},
    {gpio2_io03_rw, "GPIO2_IO03 (IO2_03)", &wflag},
    {gpio2_io04_rw, "GPIO2_IO04 (IO2_04)", &wflag},
    {gpio2_io05_rw, "GPIO2_IO05 (IO2_05)", &wflag},
    {gpio2_io06_rw, "GPIO2_IO06 (IO2_06)", &wflag},
    {gpio2_io07_rw, "GPIO2_IO07 (IO2_07)", &wflag},
    {gpio2_io08_rw, "GPIO2_IO08 (IO2_08)", &wflag},
    {gpio2_io09_rw, "GPIO2_IO09 (IO2_09)", &wflag},
    {gpio2_io10_rw, "GPIO2_IO10 (IO2_10)", &wflag},
    {gpio2_io20_rw, "GPIO2_IO20 (IO2_20)", &wflag},
    {gpio4_io00_rw, "GPIO4_IO00 (IO4_00)", &wflag},
    {gpio4_io18_rw, "GPIO4_IO18 (IO4_18)", &wflag},
    {gpio4_io19_rw, "GPIO4_IO19 (IO4_19)", &wflag},
    {gpio4_io20_rw, "GPIO4_IO20 (IO4_20)", &wflag},
    {gpio6_io08_rw, "GPIO6_IO08 (I2C_O0)", &wflag},
    {gpio6_io09_rw, "GPIO6_IO09 (I2C_O1)", &wflag},
    {gpio6_io10_rw, "GPIO6_IO10 (I2C_O2)", &wflag},
    {gpio6_io11_rw, "GPIO6_IO11 (I2C_O3)", &wflag},
    {gpio6_io12_rw, "GPIO6_IO12 (I2C_O4)", &wflag},
    {gpio6_io13_rw, "GPIO6_IO13 (I2C_O5)", &wflag},
    {gpio6_io14_rw, "GPIO6_IO14 (I2C_O6)", &wflag},
    {gpio6_io15_rw, "GPIO6_IO15 (I2C_O7)", &wflag},
    {gpio7_io08_rw, "GPIO7_IO08 (SPI_O0)", &wflag},
    {gpio7_io09_rw, "GPIO7_IO09 (SPI_O1)", &wflag},
    {gpio7_io10_rw, "GPIO7_IO10 (SPI_O2)", &wflag},
    {gpio7_io11_rw, "GPIO7_IO11 (SPI_O3)", &wflag},
    {gpio7_io12_rw, "GPIO7_IO12 (SPI_O4)", &wflag},
    {gpio7_io13_rw, "GPIO7_IO13 (SPI_O5)", &wflag},
    {gpio7_io14_rw, "GPIO7_IO14 (SPI_O6)", &wflag},
    {gpio7_io15_rw, "GPIO7_IO15 (SPI_O7)", &wflag},
    {back2, "back", &wflag},
};


#if 0
static menu_t gpio_out_menu[] = {
    {gpio_out_ctrl_port1, "GPIO PORT1"},
    {gpio_out_ctrl_port2, "GPIO PORT2"},
    {gpio_out_ctrl_port3, "GPIO PORT3"},
    {gpio_out_ctrl_port4, "GPIO PORT4"},
    // {gpio_out_ctrl_port5, "GPIO PORT5"},
    {gpio_out_ctrl_port6, "GPIO PORT6(I2C-IO)"},
    {gpio_out_ctrl_port7, "GPIO PORT7(SPI-IO)"},
    {back, "back"},
};

static menu_t gpio_in_menu[] = {
    {gpio_input_monitor, "GPIO MONITORING"},
    {gpio_in_ctrl_port1, "GPIO PORT1"},
    {gpio_in_ctrl_port2, "GPIO PORT2"},
    {gpio_in_ctrl_port3, "GPIO PORT3"},
    {gpio_in_ctrl_port4, "GPIO PORT4"},
    // {gpio_in_ctrl_port5, "GPIO PORT5"},
    {gpio_in_ctrl_port6, "GPIO PORT6(I2C-IO)"},
    {gpio_in_ctrl_port7, "GPIO PORT7(SPI-IO)"},
    {back, "back"},
};
#endif

int gpio_in_ctl(void)
{
    char *des = "GPIO INPUT TEST MENU";
    menu_args_exec(gpio_in_menu, sizeof(gpio_in_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    return 0;
}

int gpio_out_ctl(void)
{
    char *des = "GPIO OUTPUT TEST MENU";
    menu_args_exec(gpio_out_menu, sizeof(gpio_out_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    return 0;
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

    memset(gpio_stat, 0x00, sizeof(gpio_stat));

    for (port = GPIO_PORT1; port < MAX_GPIO_PORT; port++) {
        for (pin = GPIO_PIN0; pin < MAX_GPIO_PIN; pin++) {
            if (port == GPIO_PORT1 &&
                (pin != GPIO_PIN7 && pin != GPIO_PIN8 && pin != GPIO_PIN13 && pin != GPIO_PIN14))
                continue;

            if (port == GPIO_PORT2 &&
                (pin != GPIO_PIN0 && pin != GPIO_PIN1 && pin != GPIO_PIN2 && pin != GPIO_PIN3 && pin != GPIO_PIN4 &&
                    pin != GPIO_PIN5 && pin != GPIO_PIN6 && pin != GPIO_PIN7 && pin != GPIO_PIN8 &&
                    pin != GPIO_PIN9 && pin != GPIO_PIN10 && pin != GPIO_PIN20))
                continue;

            if (port == GPIO_PORT3)
                continue;

            if (port == GPIO_PORT4 &&
                (pin != GPIO_PIN0 && pin != GPIO_PIN18 && pin != GPIO_PIN19 && pin != GPIO_PIN20))
                continue;

            if (port == GPIO_PORT5)
                continue;

            if (port == GPIO_PORT6 &&
                (pin > GPIO_PIN7))
                continue;

            if (port == GPIO_PORT7 &&
                (pin > GPIO_PIN7))
                continue;

            sig = gpio_read(port, pin);
            if (sig < 0)
                continue;
            gpio_stat[port][pin].init_flag = true;
            //gpio_stat[port][pin].inval = sig;
            gpio_stat[port][pin].outval = sig;
        }
    }

    pthread_mutex_init(&gpio_lock, NULL);

    if (pthread_create(&det_thread, NULL, (void *)gpio_det_thread, NULL) < 0)
        exit(1);

    // if (pthread_create(&cpld_det_thread, NULL, (void *)gpio_cpld_det_thread, NULL) < 0)
    //     exit(1);

    printf("GPIO Init Ok.\n");
}
