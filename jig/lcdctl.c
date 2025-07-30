#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include "lcdctl.h"

#define MAX_LCD_MENU_DEPTH     2
static WINDOW *pr_win_lcd[MAX_LCD_MENU_DEPTH];
static int pr_win_lcd_depth = 0;

static void uart_rx_thread(void)
{
    //struct pollfd uarts_poll[sizeof(uarts) / sizeof(uart_t)];
    int i, uarts_num = sizeof(uarts) / sizeof(uart_t);
    char tmp[MAX_UART_XFER_SIZE];
    int ret;

    for (i = 0; i < uarts_num; i++) {
        uarts_poll[i].fd = uarts[i].fd;
        uarts_poll[i].events = POLLIN;
    }

    while (1) {
        if (poll(uarts_poll, uarts_num, 1000) <= 0)
            continue;

        for (i = 0; i < uarts_num; i++) {
            if (uarts_poll[i].revents & POLLIN) {
                usleep(100 * 1000);
                memset(tmp, 0x00, sizeof(tmp));
                if ((ret = read_uart(&uarts[i], tmp, sizeof(tmp))) <= 0)
                    continue;

                switch (i) {
                case UART_MXC1:
#ifdef UART_DEBUG
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART MXC1][READ]: \n");
                    uart_hex_print(tmp, ret);
#endif
                    break;
                case UART_MXC3:
#ifdef UART_DEBUG
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART MXC3][READ]: \n");
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(tmp, ret);
#endif
                    break;
                case UART_SERIAL1:
#ifdef UART_DEBUG
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART SERIAL1][READ]: \n");
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(tmp, ret);
#endif
                    break;
                case UART_SERIAL2:
#ifdef UART_DEBUG
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART SERIAL2][READ]: \n");
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(tmp, ret);
#endif
                    break;
                case UART_SERIAL3:
#ifdef UART_DEBUG
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART SERIAL3][READ]: \n");
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(tmp, ret);
#endif
                    break;
                case UART_SERIAL4:
#ifdef UART_DEBUG
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[UART SERIAL4][READ]: \n");
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(tmp, ret);
#endif
                    break;
                default:
                    break;
                }
            }

            if (uarts_poll[i].revents & POLLOUT) {
                char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                int ret;
                char *port_name = NULL;

                switch (i) {
                case UART_MXC1:
                    port_name = "UART MXC1";
                    break;
                case UART_MXC3:
                    port_name = "UART MXC3";
                    break;
                case UART_SERIAL1:
                    port_name = "UART SERIAL1";
                    break;
                case UART_SERIAL2:
                    port_name = "UART SERIAL2";
                    break;
                case UART_SERIAL3:
                    port_name = "UART SERIAL3";
                    break;
                case UART_SERIAL4:
                    port_name = "UART SERIAL4";
                    break;
                }

                ret = write_uart(&uarts[i], str, strlen(str));
                if (ret < 0) {
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[%s][WRITE]: error!\n", port_name);
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                } else {
                    wattron(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    pr_win(pr_win_uart[pr_win_uart_depth], "[%s][WRITE]: \n", port_name);
                    wattroff(pr_win_uart[pr_win_uart_depth], COLOR_PAIR(1));
                    uart_hex_print(str, strlen(str));
                    uarts_poll[i].events &= ~POLLOUT;
                }
            }
        }
    }
}

static int uart_speed_9600(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_9600;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 9600 bps\n", port_name);

    return 0;
}

static int uart_speed_19200(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_19200;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 19200 bps\n", port_name);

    return 0;
}

static int uart_speed_38400(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_38400;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 38400 bps\n", port_name);

    return 0;
}

static int uart_speed_57600(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_57600;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 57600 bps\n", port_name);

    return 0;
}

static int uart_speed_115200(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_115200;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 115200 bps\n", port_name);

    return 0;
}

static int uart_speed_230400(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_230400;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 230400 bps\n", port_name);

    return 0;
}

static int uart_speed_460800(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_460800;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 460800 bps\n", port_name);

    return 0;
}

static int uart_speed_921600(void *port)
{
    int *p = (int *)port;
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    }

    uarts[*p].baud = BAUD_921600;

    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: error\n", port_name);
    else
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][SPEED]: 921600 bps\n", port_name);

    return 0;
}

static int set_speed(void *port)
{
    menu_args_t spd_menu[] = {
        {uart_speed_9600, "9600", port},
        {uart_speed_19200, "19200", port},
        {uart_speed_38400, "38400", port},
        {uart_speed_57600, "57600", port},
        {uart_speed_115200, "115200", port},
        {uart_speed_230400, "230400", port},
        {uart_speed_460800, "460800", port},
        {uart_speed_921600, "921600", port},
        {back2, "back", port},
    };

    char *des = "UART Baudrate Control Menu";
    pr_win_uart_depth++;
    menu_args_exec(spd_menu, sizeof(spd_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int uart_tx_test(void *port)
{
    int *p = (int *)port;
    //char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int ret;
    char *port_name = NULL;

    switch (*p) {
    case UART_MXC1:
        port_name = "UART MXC1";
        break;
    case UART_MXC3:
        port_name = "UART MXC3";
        break;
    case UART_SERIAL1:
        port_name = "UART SERIAL1";
        break;
    case UART_SERIAL2:
        port_name = "UART SERIAL2";
        break;
    case UART_SERIAL3:
        port_name = "UART SERIAL3";
        break;
    case UART_SERIAL4:
        port_name = "UART SERIAL4";
        break;
    default:
        return -1;
    }

    uarts_poll[*p].events |= POLLOUT;
#if 0
    ret = write_uart(&uarts[*p], str, strlen(str));
    if (ret < 0)
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][WRITE]: error!\n", port_name);
    else {
        pr_win(pr_win_uart[pr_win_uart_depth], "[%s][WRITE]: \n", port_name);
        uart_hex_print(str, strlen(str));
    }
#endif

    return 0;
}

static int mxc_uart1(void)
{
    int port = UART_MXC1;
    menu_args_t muart1_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART MXC1 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(muart1_menu, sizeof(muart1_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int mxc_uart3(void)
{
    int port = UART_MXC3;
    menu_args_t muart3_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART MXC3 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(muart3_menu, sizeof(muart3_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int serial_uart1(void)
{
    int port = UART_SERIAL1;
    menu_args_t suart1_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART SERIAL1 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(suart1_menu, sizeof(suart1_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int serial_uart2(void)
{
    int port = UART_SERIAL2;
    menu_args_t suart2_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART SERIAL2 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(suart2_menu, sizeof(suart2_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int serial_uart3(void)
{
    int port = UART_SERIAL3;
    menu_args_t suart3_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART SERIAL3 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(suart3_menu, sizeof(suart3_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static int serial_uart4(void)
{
    int port = UART_SERIAL4;
    menu_args_t suart4_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "UART SERIAL4 Control Menu";

    pr_win_uart_depth++;
    menu_args_exec(suart4_menu, sizeof(suart4_menu) / sizeof(menu_args_t), des, &pr_win_uart[pr_win_uart_depth]);
    pr_win_uart_depth--;
}

static menu_t uart_menus[] = {
    {mxc_uart1, "SHOW CUBE"},
    {mxc_uart3, "SHOW TERMINAL"},
    {serial_uart1, "TOUCH CALIBRATION"},
    {serial_uart2, ""},
    {serial_uart3, "SERIAL UART3(AX99100)"},
    {serial_uart4, "SERIAL UART4(AX99100)"},
    {back, "back"},
};

int lcd_ctl(void)
{
    char *des = "LCD Control Menu";
    menu_exec(uart_menus, sizeof(uart_menus) / sizeof(menu_t), des, &pr_win_lcd[pr_win_lcd_depth]);
}

void lcd_init(void)
{
    int i;
    pthread_t rx_thread;

    for (i = 0; i < sizeof(uarts) / sizeof(uart_t); i++) {
        if (open_uart(&uarts[i]) < 0) {
            printf("uart_init() fail\n");
            exit(1);
        }
    }

    if (pthread_create(&rx_thread, NULL, (void *)uart_rx_thread, NULL) < 0)
        exit(1);

    printf("UART Init Ok.\n");
}