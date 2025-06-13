#include "gpioctl.h"
#include <linux/gpio.h>

static gpiochip_t gpios[MAX_GPIO_PORT];
static gpiostat_t gpio_stat[MAX_GPIO_PORT][MAX_GPIO_PIN];
static int gpio_input_monitoring;

static menu_t gpio_menu[] = {
    {gpio_ctrl_port1, "GPIO1"},
    {gpio_ctrl_port2, "GPIO2"},
    {gpio_ctrl_port3, "GPIO3"},
    {gpio_ctrl_port4, "GPIO4"},
    {gpio_ctrl_port5, "GPIO5"},
    {gpio_ctrl_port6, "GPIO6(I2C-IO)"},
    {gpio_ctrl_port7, "GPIO7(SPI-IO)"},
};

static menu_args_t gpio_port1_menu[] = {
    {gpio_ctrl_pin0, "GPIO1_IO0"}
};

static int gpio_read(int port, int pin)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    req.lineoffsets[0] = pin;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;

    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        return -1;
    if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        close(req.fd);
        return -1;
    }
    close(req.fd);
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

    if (ioctl(gpios[port].fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        return -1;
    if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
        close(req.fd);
        return -1;
    }
    close(req.fd);
    return data.values[0];
}

static int __gpio_input_monitor(void *args)
{
    int *onoff = (int *)args;

    gpio_input_monitoring = *onoff;

    return printf("\n\n\t[GPIO INPUT MONITORING]: %s\n\n", gpio_input_monitoring ? "ON" : "OFF");
}

static void gpio_input_monitor(void)
{
    int on = 1, off = 0;
    menu_args_t input_monitor_menus[] = {
        {__gpio_input_monitor, "ON", &on},
        {__gpio_input_monitor, "OFF", &off},
    };

    char *des = "\tSelect Input Monitoring On/Off";
    menu_args_print(input_monitor_menus, sizeof(input_monitor_menus) / sizeof(menu_args_t), des);
}

static void gpio_det_water_pwr_status(void)
{
    int ret = gpio_read(GPIO_PORT3, GPIO_PIN25);
    printf("\n\n\t[WATER SENSOR POWER STATUS] : %s\n\n", ret ? "HIGH" : "LOW");
}

static void gpio_det_lte_pwr_status(void)
{
    int ret = gpio_read(GPIO_PORT5, GPIO_PIN4);
    printf("\n\n\t[LTE MODEM POWER STATUS] : %s\n\n", ret ? "HIGH" : "LOW");
}

static void gpio_det_lte_act_status(void)
{
    int ret = gpio_read(GPIO_PORT5, GPIO_PIN5);
    printf("\n\n\t[LTE MODEM ACTIVE STATUS] : %s\n\n", ret ? "HIGH" : "LOW");
}

static int __gpio_ctl_water_pwr(void *args)
{
    int *signal = (int *)args;
    int ret = gpio_write(GPIO_PORT3, GPIO_PIN24, *signal);
    printf("\n\n\t[WATER SENSOR RELAY POWER] : %s\n\n", ret ? "HIGH" : "LOW");
    return ret;
}

static void gpio_ctl_water_pwr(void)
{
    int high = 1, low = 0;
    menu_args_t ctl_water_pwr_menus[] = {
        {__gpio_ctl_water_pwr, "HIGH", &high},
        {__gpio_ctl_water_pwr, "LOW", &low},
    };

    char *des = "\t\t Select Signal";
    menu_args_print(ctl_water_pwr_menus, sizeof(ctl_water_pwr_menus) / sizeof(menu_args_t), des);

}

static int __gpio_ctl_lte_pwr(void *args)
{
    int *signal = (int *)args;
    int ret = gpio_write(GPIO_PORT3, GPIO_PIN20, *signal);
    printf("\n\n\t[LTE MODEM RESET] : %s\n\n", ret ? "HIGH" : "LOW");
    return ret;
}

static void gpio_ctl_lte_pwr(void)
{
    int high = 1, low = 0;
    menu_args_t ctl_lte_pwr_menus[] = {
        {__gpio_ctl_lte_pwr, "HIGH", &high},
        {__gpio_ctl_lte_pwr, "LOW", &low},
    };

    char *des = "\t\t Select Signal";
    menu_args_print(ctl_lte_pwr_menus, sizeof(ctl_lte_pwr_menus) / sizeof(menu_args_t), des);

}

static int __gpio_ctl_usb_hub1_pwr(void *args)
{
    int *signal = (int *)args;
    int ret = gpio_write(GPIO_PORT3, GPIO_PIN21, *signal);
    printf("\n\n\t[USB HUB IC(USB2514B) RESET] : %s\n\n", ret ? "HIGH" : "LOW");
    return ret;
}

static void gpio_ctl_usb_hub1_pwr(void)
{
    int high = 1, low = 0;
    menu_args_t ctl_usb_hub1_pwr_menus[] = {
        {__gpio_ctl_usb_hub1_pwr, "HIGH", &high},
        {__gpio_ctl_usb_hub1_pwr, "LOW", &low},
    };

    char *des = "\t\t Select Signal";
    menu_args_print(ctl_usb_hub1_pwr_menus, sizeof(ctl_usb_hub1_pwr_menus) / sizeof(menu_args_t), des);

}

static int __gpio_ctl_usb_hub2_pwr(void *args)
{
    int *signal = (int *)args;
    int ret = gpio_write(GPIO_PORT3, GPIO_PIN22, *signal);
    printf("\n\n\t[USB HUB IC(GL850G) RESET] : %s\n\n", ret ? "HIGH" : "LOW");
    return ret;
}

static void gpio_ctl_usb_hub2_pwr(void)
{
    int high = 1, low = 0;
    menu_args_t ctl_usb_hub2_pwr_menus[] = {
        {__gpio_ctl_usb_hub2_pwr, "HIGH", &high},
        {__gpio_ctl_usb_hub2_pwr, "LOW", &low},
    };

    char *des = "\t\t Select Signal";
    menu_args_print(ctl_usb_hub2_pwr_menus, sizeof(ctl_usb_hub2_pwr_menus) / sizeof(menu_args_t), des);

}

static int __gpio_ctl_usb_serial_pwr(void *args)
{
    int *signal = (int *)args;
    int ret = gpio_write(GPIO_PORT3, GPIO_PIN23, *signal);
    printf("\n\n  [USB TO SERIAL IC(FT4232HL) RESET] : %s\n\n", ret ? "HIGH" : "LOW");
    return ret;
}

static void gpio_ctl_usb_serial_pwr(void)
{
    int high = 1, low = 0;
    menu_args_t ctl_usb_serial_pwr_menus[] = {
        {__gpio_ctl_usb_serial_pwr, "HIGH", &high},
        {__gpio_ctl_usb_serial_pwr, "LOW", &low},
    };

    char *des = "\t\t Select Signal";
    menu_args_print(ctl_usb_serial_pwr_menus, sizeof(ctl_usb_serial_pwr_menus) / sizeof(menu_args_t), des);
}

static menu_t gpio_input_menus[] = {
    {gpio_input_monitor, "ALL PIN MONITORING"},
    {gpio_det_water_pwr_status, "WATER SENSOR POWER STATUS"},
    {gpio_det_lte_pwr_status, "LTE MODEM POWER STATUS"},
    {gpio_det_lte_act_status, "LTE MODEM ACTIVE STATUS"},
};

static menu_t gpio_output_menus[] = {
    {gpio_ctl_water_pwr, "WATER SENSOR RELAY POWER"},
    {gpio_ctl_lte_pwr, "LTE MODEM RESET"},
    {gpio_ctl_usb_hub1_pwr, "USB HUB IC(USB2514B) RESET"},
    {gpio_ctl_usb_hub2_pwr, "USB HUB IC(GL850G) RESET"},
    {gpio_ctl_usb_serial_pwr, "USB TO SERIAL IC(FT4232HL) RESET"},
};

static void gpio_input(void)
{
    char *des = "\t    GPIO Input Pin Menu";
    menu_print(gpio_input_menus, sizeof(gpio_input_menus) / sizeof(menu_t), des);
}

static void gpio_output(void)
{
    char *des = "\t    GPIO Output Pin Menu";
    menu_print(gpio_output_menus, sizeof(gpio_output_menus) / sizeof(menu_t), des);
}

static void gpio_det_thread(void)
{
    int port, pin;
    int old, new;

    while (1) {
        sleep(1);

        if (!gpio_input_monitoring)
            continue;

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

        for (port = GPIO_PORT1; port < MAX_GPIO_PORT; port++) {
            for (pin = GPIO_PIN0; pin < MAX_GPIO_PIN; pin++) {
                if (gpio_read(port, pin) < 0)
                    continue;
            }
        }
    }
}

static int gpio_ctrl_port1(void)
{
}

static int gpio_ctrl_port2(void)
{
}

static int gpio_ctrl_port3(void)
{
}

static int gpio_ctrl_port4(void)
{
}

static int gpio_ctrl_port5(void)
{
}

static int gpio_ctrl_port6(void)
{
}

static int gpio_ctrl_port7(void)
{
}

static menu_t gpio_menu[] = {
    {gpio_ctrl_port1, "GPIO PORT1"},
    {gpio_ctrl_port2, "GPIO PORT2"},
    {gpio_ctrl_port3, "GPIO PORT3"},
    {gpio_ctrl_port4, "GPIO PORT4"},
    {gpio_ctrl_port5, "GPIO PORT5"},
    {gpio_ctrl_port6, "GPIO PORT6(I2C-IO)"},
    {gpio_ctrl_port7, "GPIO PORT7(SPI-IO)"},
};

void gpio_ctrl(void)
{
    char *des = "\t     GPIO Control Menu";
    menu_exec(gpio_menu, sizeof(gpio_menu) / sizeof(menu_t), des);
}

void gpio_init(void)
{
    int i, port, pin, sig;
    pthread_t det_thread;

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
            sig = gpio_read(port, pin);
            if (sig < 0)
                continue;
            gpio_stat[port][pin].init_flag = true;
            gpio_stat[port][pin].data = sig;
        }
    }

    if (pthread_create(&det_thread, NULL, (void *)gpio_det_thread, NULL) < 0)
        exit(1);

    printf("GPIO Init Ok.\n");
}
