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

static int gpio_input_monitor(void)
{
    int on = 1, off = 0;
    menu_args_t input_monitor_menus[] = {
        {__gpio_input_monitor, "ON", &on},
        {__gpio_input_monitor, "OFF", &off},
        {back2, "back", &on}
    };

    char *des = "Select Input Monitoring(On/Off)";
    pr_win_gpio_depth++;
    menu_args_exec(input_monitor_menus, sizeof(input_monitor_menus) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
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
        new = gpio_read(GPIO_PORT3, GPIO_PIN19);
        if (new) {
            if (old != new) {
                old = new;
                gpio_write(GPIO_PORT2, GPIO_PIN0, 0);   // act led on
                gpio_write(GPIO_PORT3, GPIO_PIN20, 0);
                pr_win(pr_win_gpio[pr_win_gpio_depth], "ID[%d] = %s\n", id, old ? "active" : "inactive");
            }
        } else {
            if (old != new) {
                old = new;
                gpio_write(GPIO_PORT2, GPIO_PIN0, 1);   // act led off
                gpio_write(GPIO_PORT3, GPIO_PIN20, 1);
                pr_win(pr_win_gpio[pr_win_gpio_depth], "ID[%d] = %s\n", id, old ? "active" : "inactive");
            }
        }
        usleep(100 * 1000);
    }
}

static void gpio_det_thread(void)
{
    int port, pin;
    int old, new;
    int x, y;

    while (1) {
        sleep(1);

        if (!gpio_input_monitoring)
            continue;
#if 0
        water_sens_pwr_status = gpio_read(GPIO_PORT3, GPIO_PIN25);
        lte_modem_pwr_status = gpio_read(GPIO_PORT5, GPIO_PIN4);
        lte_modem_act_status = gpio_read(GPIO_PORT5, GPIO_PIN5);

        if (water_sens_pwr_status != old_water_sens_pwr_status) {
            printf("\n\n\t[WATER SENSOR POWER STATUS] : %s\n\n", water_sens_pwr_status ? "HIGH" : "LOW");
            old_water_sens_pwr_status = water_sens_pwr_status;
        }
        if (lte_modem_pwr_status != old_lte_modem_pwr_status) {
            printf("\n\n\t[LTE MODEM POWER STATUS] : %s\n\n", lte_modem_pwr_status ? "HIGH" : "LOW");
            old_lte_modem_pwr_status = lte_modem_pwr_status;
        }
        if (lte_modem_act_status != old_lte_modem_act_status) {
            printf("\n\n\t[LTE MODEM ACTIVE STATUS] : %s\n\n", lte_modem_act_status ? "HIGH" : "LOW");
            old_lte_modem_act_status = lte_modem_act_status;
        }
#endif

        for (port = GPIO_PORT1; port < MAX_GPIO_PORT; port++) {
            for (pin = GPIO_PIN0; pin < MAX_GPIO_PIN; pin++) {
                if (!gpio_stat[port][pin].init_flag)
                    continue;

                /* CPLD IN control pin */
                if (port == GPIO_PORT3 && pin == GPIO_PIN20)
                    continue;

                /* CPLD MS LED control pin */
                if (port == GPIO_PORT1 && pin == GPIO_PIN8)
                    continue;

                /* CPLD ACT LED control pin */
                if (port == GPIO_PORT2 && pin == GPIO_PIN0)
                    continue;

                new = gpio_read(port, pin);
                if (new < 0) {
                    pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][READ]: error!\n", port + 1, pin);
                    continue;
                }

                old = gpio_stat[port][pin].inval;
                if (old != new) {
                    gpio_stat[port][pin].inval = new;
                    pr_win(pr_win_gpio[pr_win_gpio_depth], "[GPIO%d_IO%02d][READ]: \"%s\"\n", port + 1, pin, new ? "HIGH" : "LOW");
                }
                usleep(1 * 1000);
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
    char *des = "GPIO PORT1 Control Menu";
    int port = GPIO_PORT1;

    menu_args_t gpio_port1_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO1_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO1_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO1_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO1_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO1_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO1_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO1_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO1_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO1_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO1_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO1_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO1_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO1_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO1_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO1_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO1_IO15", &port},
        {gpio_out_ctrl_pin16, "GPIO1_IO16", &port},
        {gpio_out_ctrl_pin17, "GPIO1_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO1_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO1_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO1_IO20", &port},
        {gpio_out_ctrl_pin21, "GPIO1_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO1_IO22", &port},
        {gpio_out_ctrl_pin23, "GPIO1_IO23", &port},
        {gpio_out_ctrl_pin24, "GPIO1_IO24", &port},
        {gpio_out_ctrl_pin25, "GPIO1_IO25", &port},
        {gpio_out_ctrl_pin26, "GPIO1_IO26", &port},
        {gpio_out_ctrl_pin27, "GPIO1_IO27", &port},
        {gpio_out_ctrl_pin28, "GPIO1_IO28", &port},
        {gpio_out_ctrl_pin29, "GPIO1_IO29", &port},
        {gpio_out_ctrl_pin30, "GPIO1_IO30", &port},
        {gpio_out_ctrl_pin31, "GPIO1_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port1_menu, sizeof(gpio_port1_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port2(void)
{
    char *des = "GPIO PORT2 Control Menu";
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
        {gpio_out_ctrl_pin11, "GPIO2_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO2_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO2_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO2_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO2_IO15", &port},
        {gpio_out_ctrl_pin16, "GPIO2_IO16", &port},
        {gpio_out_ctrl_pin17, "GPIO2_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO2_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO2_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO2_IO20", &port},
        {gpio_out_ctrl_pin21, "GPIO2_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO2_IO22", &port},
        {gpio_out_ctrl_pin23, "GPIO2_IO23", &port},
        {gpio_out_ctrl_pin24, "GPIO2_IO24", &port},
        {gpio_out_ctrl_pin25, "GPIO2_IO25", &port},
        {gpio_out_ctrl_pin26, "GPIO2_IO26", &port},
        {gpio_out_ctrl_pin27, "GPIO2_IO27", &port},
        {gpio_out_ctrl_pin28, "GPIO2_IO28", &port},
        {gpio_out_ctrl_pin29, "GPIO2_IO29", &port},
        {gpio_out_ctrl_pin30, "GPIO2_IO30", &port},
        {gpio_out_ctrl_pin31, "GPIO2_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port2_menu, sizeof(gpio_port2_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port3(void)
{
    char *des = "GPIO PORT3 Control Menu";
    int port = GPIO_PORT3;

    menu_args_t gpio_port3_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO3_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO3_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO3_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO3_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO3_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO3_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO3_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO3_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO3_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO3_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO3_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO3_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO3_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO3_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO3_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO3_IO15", &port},
        {gpio_out_ctrl_pin16, "GPIO3_IO16", &port},
        {gpio_out_ctrl_pin17, "GPIO3_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO3_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO3_IO19(PLD-OUT)", &port},
        {gpio_out_ctrl_pin20, "GPIO3_IO20(PLD-IN)", &port},
        {gpio_out_ctrl_pin21, "GPIO3_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO3_IO22", &port},
        {gpio_out_ctrl_pin23, "GPIO3_IO23", &port},
        {gpio_out_ctrl_pin24, "GPIO3_IO24", &port},
        {gpio_out_ctrl_pin25, "GPIO3_IO25", &port},
        {gpio_out_ctrl_pin26, "GPIO3_IO26", &port},
        {gpio_out_ctrl_pin27, "GPIO3_IO27", &port},
        {gpio_out_ctrl_pin28, "GPIO3_IO28", &port},
        {gpio_out_ctrl_pin29, "GPIO3_IO29", &port},
        {gpio_out_ctrl_pin30, "GPIO3_IO30", &port},
        {gpio_out_ctrl_pin31, "GPIO3_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port3_menu, sizeof(gpio_port3_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port4(void)
{
    char *des = "GPIO PORT4 Control Menu";
    int port = GPIO_PORT4;

    menu_args_t gpio_port4_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO4_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO4_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO4_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO4_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO4_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO4_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO4_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO4_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO4_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO4_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO4_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO4_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO4_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO4_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO4_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO4_IO15", &port},
        {gpio_out_ctrl_pin16, "GPIO4_IO16", &port},
        {gpio_out_ctrl_pin17, "GPIO4_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO4_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO4_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO4_IO20", &port},
        {gpio_out_ctrl_pin21, "GPIO4_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO4_IO22", &port},
        {gpio_out_ctrl_pin23, "GPIO4_IO23", &port},
        {gpio_out_ctrl_pin24, "GPIO4_IO24", &port},
        {gpio_out_ctrl_pin25, "GPIO4_IO25", &port},
        {gpio_out_ctrl_pin26, "GPIO4_IO26", &port},
        {gpio_out_ctrl_pin27, "GPIO4_IO27", &port},
        {gpio_out_ctrl_pin28, "GPIO4_IO28", &port},
        {gpio_out_ctrl_pin29, "GPIO4_IO29", &port},
        {gpio_out_ctrl_pin30, "GPIO4_IO30", &port},
        {gpio_out_ctrl_pin31, "GPIO4_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port4_menu, sizeof(gpio_port4_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port5(void)
{
    char *des = "GPIO PORT5 Control Menu";
    int port = GPIO_PORT5;

    menu_args_t gpio_port5_menu[] = {
        {gpio_out_ctrl_pin0, "GPIO5_IO00", &port},
        {gpio_out_ctrl_pin1, "GPIO5_IO01", &port},
        {gpio_out_ctrl_pin2, "GPIO5_IO02", &port},
        {gpio_out_ctrl_pin3, "GPIO5_IO03", &port},
        {gpio_out_ctrl_pin4, "GPIO5_IO04", &port},
        {gpio_out_ctrl_pin5, "GPIO5_IO05", &port},
        {gpio_out_ctrl_pin6, "GPIO5_IO06", &port},
        {gpio_out_ctrl_pin7, "GPIO5_IO07", &port},
        {gpio_out_ctrl_pin8, "GPIO5_IO08", &port},
        {gpio_out_ctrl_pin9, "GPIO5_IO09", &port},
        {gpio_out_ctrl_pin10, "GPIO5_IO10", &port},
        {gpio_out_ctrl_pin11, "GPIO5_IO11", &port},
        {gpio_out_ctrl_pin12, "GPIO5_IO12", &port},
        {gpio_out_ctrl_pin13, "GPIO5_IO13", &port},
        {gpio_out_ctrl_pin14, "GPIO5_IO14", &port},
        {gpio_out_ctrl_pin15, "GPIO5_IO15", &port},
        {gpio_out_ctrl_pin16, "GPIO5_IO16", &port},
        {gpio_out_ctrl_pin17, "GPIO5_IO17", &port},
        {gpio_out_ctrl_pin18, "GPIO5_IO18", &port},
        {gpio_out_ctrl_pin19, "GPIO5_IO19", &port},
        {gpio_out_ctrl_pin20, "GPIO5_IO20", &port},
        {gpio_out_ctrl_pin21, "GPIO5_IO21", &port},
        {gpio_out_ctrl_pin22, "GPIO5_IO22", &port},
        {gpio_out_ctrl_pin23, "GPIO5_IO23", &port},
        {gpio_out_ctrl_pin24, "GPIO5_IO24", &port},
        {gpio_out_ctrl_pin25, "GPIO5_IO25", &port},
        {gpio_out_ctrl_pin26, "GPIO5_IO26", &port},
        {gpio_out_ctrl_pin27, "GPIO5_IO27", &port},
        {gpio_out_ctrl_pin28, "GPIO5_IO28", &port},
        {gpio_out_ctrl_pin29, "GPIO5_IO29", &port},
        {gpio_out_ctrl_pin30, "GPIO5_IO30", &port},
        {gpio_out_ctrl_pin31, "GPIO5_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port5_menu, sizeof(gpio_port5_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_out_ctrl_port6(void)
{
    char *des = "GPIO PORT6 Control Menu";
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
    char *des = "GPIO PORT7 Control Menu";
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
    char *des = "GPIO PORT1 Control Menu";
    int port = GPIO_PORT1;

    menu_args_t gpio_port1_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO1_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO1_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO1_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO1_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO1_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO1_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO1_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO1_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO1_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO1_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO1_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO1_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO1_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO1_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO1_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO1_IO15", &port},
        {gpio_in_ctrl_pin16, "GPIO1_IO16", &port},
        {gpio_in_ctrl_pin17, "GPIO1_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO1_IO18", &port},
        {gpio_in_ctrl_pin19, "GPIO1_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO1_IO20", &port},
        {gpio_in_ctrl_pin21, "GPIO1_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO1_IO22", &port},
        {gpio_in_ctrl_pin23, "GPIO1_IO23", &port},
        {gpio_in_ctrl_pin24, "GPIO1_IO24", &port},
        {gpio_in_ctrl_pin25, "GPIO1_IO25", &port},
        {gpio_in_ctrl_pin26, "GPIO1_IO26", &port},
        {gpio_in_ctrl_pin27, "GPIO1_IO27", &port},
        {gpio_in_ctrl_pin28, "GPIO1_IO28", &port},
        {gpio_in_ctrl_pin29, "GPIO1_IO29", &port},
        {gpio_in_ctrl_pin30, "GPIO1_IO30", &port},
        {gpio_in_ctrl_pin31, "GPIO1_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port1_menu, sizeof(gpio_port1_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port2(void)
{
    char *des = "GPIO PORT2 Control Menu";
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
        {gpio_in_ctrl_pin11, "GPIO2_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO2_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO2_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO2_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO2_IO15", &port},
        {gpio_in_ctrl_pin16, "GPIO2_IO16", &port},
        {gpio_in_ctrl_pin17, "GPIO2_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO2_IO18", &port},
        {gpio_in_ctrl_pin19, "GPIO2_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO2_IO20", &port},
        {gpio_in_ctrl_pin21, "GPIO2_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO2_IO22", &port},
        {gpio_in_ctrl_pin23, "GPIO2_IO23", &port},
        {gpio_in_ctrl_pin24, "GPIO2_IO24", &port},
        {gpio_in_ctrl_pin25, "GPIO2_IO25", &port},
        {gpio_in_ctrl_pin26, "GPIO2_IO26", &port},
        {gpio_in_ctrl_pin27, "GPIO2_IO27", &port},
        {gpio_in_ctrl_pin28, "GPIO2_IO28", &port},
        {gpio_in_ctrl_pin29, "GPIO2_IO29", &port},
        {gpio_in_ctrl_pin30, "GPIO2_IO30", &port},
        {gpio_in_ctrl_pin31, "GPIO2_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port2_menu, sizeof(gpio_port2_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port3(void)
{
    char *des = "GPIO PORT3 Control Menu";
    int port = GPIO_PORT3;

    menu_args_t gpio_port3_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO3_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO3_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO3_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO3_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO3_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO3_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO3_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO3_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO3_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO3_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO3_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO3_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO3_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO3_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO3_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO3_IO15", &port},
        {gpio_in_ctrl_pin16, "GPIO3_IO16", &port},
        {gpio_in_ctrl_pin17, "GPIO3_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO3_IO18", &port},
        {gpio_in_ctrl_pin19, "GPIO3_IO19(PLD-OUT)", &port},
        {gpio_in_ctrl_pin20, "GPIO3_IO20(PLD-IN)", &port},
        {gpio_in_ctrl_pin21, "GPIO3_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO3_IO22", &port},
        {gpio_in_ctrl_pin23, "GPIO3_IO23", &port},
        {gpio_in_ctrl_pin24, "GPIO3_IO24", &port},
        {gpio_in_ctrl_pin25, "GPIO3_IO25", &port},
        {gpio_in_ctrl_pin26, "GPIO3_IO26", &port},
        {gpio_in_ctrl_pin27, "GPIO3_IO27", &port},
        {gpio_in_ctrl_pin28, "GPIO3_IO28", &port},
        {gpio_in_ctrl_pin29, "GPIO3_IO29", &port},
        {gpio_in_ctrl_pin30, "GPIO3_IO30", &port},
        {gpio_in_ctrl_pin31, "GPIO3_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port3_menu, sizeof(gpio_port3_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port4(void)
{
    char *des = "GPIO PORT4 Control Menu";
    int port = GPIO_PORT4;

    menu_args_t gpio_port4_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO4_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO4_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO4_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO4_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO4_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO4_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO4_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO4_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO4_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO4_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO4_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO4_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO4_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO4_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO4_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO4_IO15", &port},
        {gpio_in_ctrl_pin16, "GPIO4_IO16", &port},
        {gpio_in_ctrl_pin17, "GPIO4_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO4_IO18", &port},
        {gpio_in_ctrl_pin19, "GPIO4_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO4_IO20", &port},
        {gpio_in_ctrl_pin21, "GPIO4_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO4_IO22", &port},
        {gpio_in_ctrl_pin23, "GPIO4_IO23", &port},
        {gpio_in_ctrl_pin24, "GPIO4_IO24", &port},
        {gpio_in_ctrl_pin25, "GPIO4_IO25", &port},
        {gpio_in_ctrl_pin26, "GPIO4_IO26", &port},
        {gpio_in_ctrl_pin27, "GPIO4_IO27", &port},
        {gpio_in_ctrl_pin28, "GPIO4_IO28", &port},
        {gpio_in_ctrl_pin29, "GPIO4_IO29", &port},
        {gpio_in_ctrl_pin30, "GPIO4_IO30", &port},
        {gpio_in_ctrl_pin31, "GPIO4_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port4_menu, sizeof(gpio_port4_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port5(void)
{
    char *des = "GPIO PORT5 Control Menu";
    int port = GPIO_PORT5;

    menu_args_t gpio_port5_menu[] = {
        {gpio_in_ctrl_pin0, "GPIO5_IO00", &port},
        {gpio_in_ctrl_pin1, "GPIO5_IO01", &port},
        {gpio_in_ctrl_pin2, "GPIO5_IO02", &port},
        {gpio_in_ctrl_pin3, "GPIO5_IO03", &port},
        {gpio_in_ctrl_pin4, "GPIO5_IO04", &port},
        {gpio_in_ctrl_pin5, "GPIO5_IO05", &port},
        {gpio_in_ctrl_pin6, "GPIO5_IO06", &port},
        {gpio_in_ctrl_pin7, "GPIO5_IO07", &port},
        {gpio_in_ctrl_pin8, "GPIO5_IO08", &port},
        {gpio_in_ctrl_pin9, "GPIO5_IO09", &port},
        {gpio_in_ctrl_pin10, "GPIO5_IO10", &port},
        {gpio_in_ctrl_pin11, "GPIO5_IO11", &port},
        {gpio_in_ctrl_pin12, "GPIO5_IO12", &port},
        {gpio_in_ctrl_pin13, "GPIO5_IO13", &port},
        {gpio_in_ctrl_pin14, "GPIO5_IO14", &port},
        {gpio_in_ctrl_pin15, "GPIO5_IO15", &port},
        {gpio_in_ctrl_pin16, "GPIO5_IO16", &port},
        {gpio_in_ctrl_pin17, "GPIO5_IO17", &port},
        {gpio_in_ctrl_pin18, "GPIO5_IO18", &port},
        {gpio_in_ctrl_pin19, "GPIO5_IO19", &port},
        {gpio_in_ctrl_pin20, "GPIO5_IO20", &port},
        {gpio_in_ctrl_pin21, "GPIO5_IO21", &port},
        {gpio_in_ctrl_pin22, "GPIO5_IO22", &port},
        {gpio_in_ctrl_pin23, "GPIO5_IO23", &port},
        {gpio_in_ctrl_pin24, "GPIO5_IO24", &port},
        {gpio_in_ctrl_pin25, "GPIO5_IO25", &port},
        {gpio_in_ctrl_pin26, "GPIO5_IO26", &port},
        {gpio_in_ctrl_pin27, "GPIO5_IO27", &port},
        {gpio_in_ctrl_pin28, "GPIO5_IO28", &port},
        {gpio_in_ctrl_pin29, "GPIO5_IO29", &port},
        {gpio_in_ctrl_pin30, "GPIO5_IO30", &port},
        {gpio_in_ctrl_pin31, "GPIO5_IO31", &port},
        { back2, "back", &port }
    };

    pr_win_gpio_depth++;
    menu_args_exec(gpio_port5_menu, sizeof(gpio_port5_menu) / sizeof(menu_args_t), des, &pr_win_gpio[pr_win_gpio_depth]);
    pr_win_gpio_depth--;
}

static int gpio_in_ctrl_port6(void)
{
    char *des = "GPIO PORT6 Control Menu";
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
    char *des = "GPIO PORT7 Control Menu";
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

static menu_t gpio_out_menu[] = {
    {gpio_out_ctrl_port1, "GPIO PORT1"},
    {gpio_out_ctrl_port2, "GPIO PORT2"},
    {gpio_out_ctrl_port3, "GPIO PORT3"},
    {gpio_out_ctrl_port4, "GPIO PORT4"},
    {gpio_out_ctrl_port5, "GPIO PORT5"},
    {gpio_out_ctrl_port6, "GPIO PORT6(I2C-IO)"},
    {gpio_out_ctrl_port7, "GPIO PORT7(SPI-IO)"},
    {back, "back"},
};

static menu_t gpio_in_menu[] = {
    {gpio_input_monitor, "GPIO Monitoring"},
    {gpio_in_ctrl_port1, "GPIO PORT1"},
    {gpio_in_ctrl_port2, "GPIO PORT2"},
    {gpio_in_ctrl_port3, "GPIO PORT3"},
    {gpio_in_ctrl_port4, "GPIO PORT4"},
    {gpio_in_ctrl_port5, "GPIO PORT5"},
    {gpio_in_ctrl_port6, "GPIO PORT6(I2C-IO)"},
    {gpio_in_ctrl_port7, "GPIO PORT7(SPI-IO)"},
    {back, "back"},
};

int gpio_in_ctrl(void)
{
    char *des = "GPIO Input Control Menu";
    menu_exec(gpio_in_menu, sizeof(gpio_in_menu) / sizeof(menu_t), des, &pr_win_gpio[pr_win_gpio_depth]);
}

int gpio_out_ctrl(void)
{
    char *des = "GPIO Output Control Menu";
    menu_exec(gpio_out_menu, sizeof(gpio_out_menu) / sizeof(menu_t), des, &pr_win_gpio[pr_win_gpio_depth]);
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
            if (port == GPIO_PORT3 && pin == GPIO_PIN20) {
                gpio_stat[port][pin].init_flag = true;
                continue;
            }

            if (port == GPIO_PORT1 && pin == GPIO_PIN8) {
                gpio_stat[port][pin].init_flag = true;
                continue;
            }

            if (port == GPIO_PORT2 && pin == GPIO_PIN0) {
                gpio_stat[port][pin].init_flag = true;
                continue;
            }

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

    if (pthread_create(&cpld_det_thread, NULL, (void *)gpio_cpld_det_thread, NULL) < 0)
        exit(1);

    printf("GPIO Init Ok.\n");
}
