#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include "uartctl.h"

#define UART_DEBUG

static uart_t uarts[] = {
    [UART_MXC1] = {
        .dev = DEV_NXP_UART1,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
    [UART_MXC3] = {
        .dev = DEV_NXP_UART3,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
    [UART_SERIAL1] = {
        .dev = DEV_SERIAL_UART1,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
    [UART_SERIAL2] = {
        .dev = DEV_SERIAL_UART2,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
    [UART_SERIAL3] = {
        .dev = DEV_SERIAL_UART3,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
    [UART_SERIAL4] = {
        .dev = DEV_SERIAL_UART4,
        .baud = BAUD_9600,
        .fd = -1,
        .sem = -1,
    },
};

static void uart_hex_print(const char *buf, int size)
{
    int x, y, i;

    for (i = 0; i < size; i++) {
        getyx(stdscr, y, x);
        mvprintw(y, x, "0x%02X ", buf[i]);
        if ((i + 1) % 16 == 0) {
            getyx(stdscr, y, x);
            mvprintw(y, x, "\n");
        }
    }
    getyx(stdscr, y, x);
    mvprintw(y, x, "\n");
}

static void uart_rx_thread(void)
{
    struct pollfd uarts_poll[sizeof(uarts) / sizeof(uart_t)];
    int i, uarts_num = sizeof(uarts) / sizeof(uart_t);
    char tmp[MAX_UART_XFER_SIZE];
    int ret;
    int x, y;

    for (i = 0; i < uarts_num; i++) {
        uarts_poll[i].fd = uarts[i].fd;
        uarts_poll[i].events = POLLIN;
    }

    while (1) {
        if (poll(uarts_poll, uarts_num, 1000) <= 0)
            continue;

        for (i = 0; i < uarts_num; i++) {
            if (uarts_poll[i].revents & POLLIN) {
                memset(tmp, 0x00, sizeof(tmp));
                if ((ret = read_uart(&uarts[i], tmp, sizeof(tmp))) <= 0)
                    continue;

                switch (i) {
                case UART_MXC1:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART MXC1][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    //lte_parser(tmp, ret);
                    break;
                case UART_MXC3:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART MXC3][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    break;
                case UART_SERIAL1:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART SERIAL1][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    break;
                case UART_SERIAL2:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART SERIAL2][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    break;
                case UART_SERIAL3:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART SERIAL3][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    break;
                case UART_SERIAL4:
#ifdef UART_DEBUG
                    getyx(stdscr, y, x);
                    mvprintw(y, 0, "[UART SERIAL4][READ] :\n");
                    uart_hex_print(tmp, ret);
                    refresh();
#endif
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static int uart_speed_9600(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 9600 bps\n", port_name);

    return 0;
}

static int uart_speed_19200(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 19200 bps\n", port_name);

    return 0;
}

static int uart_speed_38400(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 38400 bps\n", port_name);

    return 0;
}

static int uart_speed_57600(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 57600 bps\n", port_name);

    return 0;
}

static int uart_speed_115200(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 115200 bps\n", port_name);

    return 0;
}

static int uart_speed_230400(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 230400 bps\n", port_name);

    return 0;
}

static int uart_speed_460800(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 460800 bps\n", port_name);

    return 0;
}

static int uart_speed_921600(void *port)
{
    int *p = (int *)port;
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = set_uart_speed(&uarts[*p]);
    if (ret < 0)
        mvprintw(y, 0, "[%s][SPEED] : error!\n", port_name);
    else
        mvprintw(y, 0, "[%s][SPEED] : 921600 bps\n", port_name);

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

    char *des = "\t  UART Baudrate Control Menu";
    menu_args_exec(spd_menu, sizeof(spd_menu) / sizeof(menu_args_t), des);
}

static int uart_tx_test(void *port)
{
    int *p = (int *)port;
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int x, y, ret;
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

    getyx(stdscr, y, x);
    ret = write_uart(&uarts[*p], str, strlen(str));
    if (ret < 0)
        mvprintw(y, 0, "[%s][WRITE] : error!\n", port_name);
    else {
        mvprintw(y, 0, "[%s][WRITE] :\n", port_name);
        uart_hex_print(str, strlen(str));
    }

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
    char *des = "\t  UART MXC1 Control Menu";

    menu_args_exec(muart1_menu, sizeof(muart1_menu) / sizeof(menu_args_t), des);
}

static int mxc_uart3(void)
{
    int port = UART_MXC3;
    menu_args_t muart3_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "\t  UART MXC3 Control Menu";

    menu_args_exec(muart3_menu, sizeof(muart3_menu) / sizeof(menu_args_t), des);
}

static int serial_uart1(void)
{
    int port = UART_SERIAL1;
    menu_args_t suart1_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "\t  UART SERIAL1 Control Menu";

    menu_args_exec(suart1_menu, sizeof(suart1_menu) / sizeof(menu_args_t), des);
}

static int serial_uart2(void)
{
    int port = UART_SERIAL2;
    menu_args_t suart2_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "\t  UART SERIAL2 Control Menu";

    menu_args_exec(suart2_menu, sizeof(suart2_menu) / sizeof(menu_args_t), des);
}

static int serial_uart3(void)
{
    int port = UART_SERIAL3;
    menu_args_t suart3_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "\t  UART SERIAL3 Control Menu";

    menu_args_exec(suart3_menu, sizeof(suart3_menu) / sizeof(menu_args_t), des);
}

static int serial_uart4(void)
{
    int port = UART_SERIAL4;
    menu_args_t suart4_menu[] = {
        {set_speed, "SET BAUDRATE", &port},
        {uart_tx_test, "TX TEST", &port},
        {back2, "back", &port},
    };
    char *des = "\t  UART SERIAL4 Control Menu";

    menu_args_exec(suart4_menu, sizeof(suart4_menu) / sizeof(menu_args_t), des);
}

static menu_t uart_menus[] = {
    {mxc_uart1, "MXC UART1"},
    {mxc_uart3, "MXC UART3"},
    {serial_uart1, "SERIAL UART1(AX99100)"},
    {serial_uart2, "SERIAL UART2(AX99100)"},
    {serial_uart3, "SERIAL UART3(AX99100)"},
    {serial_uart4, "SERIAL UART4(AX99100)"},
    {back, "back"},
};

int uart_ctrl(void)
{
    char *des = "\t      UART Control Menu";
    menu_exec(uart_menus, sizeof(uart_menus) / sizeof(menu_t), des);
}

void uart_init(void)
{
    int i;
    pthread_t rx_thread;

    for (i = 0; i < sizeof(uarts) / sizeof(uart_t); i++) {
        if (open_uart(&uarts[i]) < 0) {
            printf("uart_init() fail\n");
            //exit(1);
        }
    }

    if (pthread_create(&rx_thread, NULL, (void *)uart_rx_thread, NULL) < 0)
        exit(1);

    printf("UART Init Ok.\n");
}