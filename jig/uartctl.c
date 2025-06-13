#include "uartctl.h"
#include "ltectl.h"

#define UART_DEBUG

static uart_t uarts[] = {
    {DEV_NXP_UART1, BAUD_9600, -1, -1},
    {DEV_NXP_UART3, BAUD_9600, -1, -1},
    {DEV_USB_UART1, BAUD_115200, -1, -1},
    {DEV_USB_UART2, BAUD_115200, -1, -1},
    {DEV_USB_UART3, BAUD_115200, -1, -1},
};

static void lte_tx_test(void)
{
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    lte_tx(str, strlen(str));
}

static void wvu_tx_test(void)
{
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    wvu_tx(str, strlen(str));
}

static void solar_tx_test(void)
{
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    solar_tx(str, strlen(str));
}

static void water1_tx_test(void)
{
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    water1_tx(str, strlen(str));
}

static void water2_tx_test(void)
{
    char *str = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    water2_tx(str, strlen(str));
}

static void uart_hex_print(const char *buf, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        printf("0x%02X ", buf[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n");
}

static void uart_rx_thread(void)
{
    struct pollfd uarts_poll[sizeof(uarts)/sizeof(uart_t)];
    int i, uarts_num = sizeof(uarts)/sizeof(uart_t);
    char tmp[MAX_UART_XFER_SIZE];

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
                uart_ctrl_t rxd;
                rxd.data = tmp;
                if (read_uart(&uarts[i], &rxd) <= 0)
                    continue;

                switch (i) {
                    case UART_LTE:
#ifdef UART_DEBUG
                        printf("\n=====[RX.%s.%s]====================\n", uarts[i].dev, "UART_LTE");
                        uart_hex_print(rxd.data, rxd.len);
#endif
                        lte_parser(rxd.data, rxd.len);
                        break;
                    case UART_WVU:
#ifdef UART_DEBUG
                        printf("\n=====[RX.%s.%s]====================\n", uarts[i].dev, "UART_WVU");
                        uart_hex_print(rxd.data, rxd.len);
#endif
                        break;
                    case UART_SOLAR:
#ifdef UART_DEBUG
                        printf("\n=====[RX.%s.%s]====================\n", uarts[i].dev, "UART_SOLAR");
                        uart_hex_print(rxd.data, rxd.len);
#endif
                        break;
                    case UART_WATER1:
#ifdef UART_DEBUG
                        printf("\n=====[RX.%s.%s]====================\n", uarts[i].dev, "UART_WATER1");
                        uart_hex_print(rxd.data, rxd.len);
#endif
                        break;
                    case UART_WATER2:
#ifdef UART_DEBUG
                        printf("\n=====[RX.%s.%s]====================\n", uarts[i].dev, "UART_WATER2");
                        uart_hex_print(rxd.data, rxd.len);
#endif
                        break;
                    default:
                        break;
                }
            }
        }
    }
}


static menu_t uart_menus[] = {
    {lte_tx_test, "UART1.LTE"},
    {wvu_tx_test, "UART3.WVU"},
    {solar_tx_test, "USB UART1.SOLAR"},
    {water1_tx_test, "USB UART2.WATER1"},
    {water2_tx_test, "USB UART3.WATER2"},
};

void uart_control(void)
{
    char *des = "\t      UART Control Menu";
    menu_print(uart_menus, sizeof(uart_menus)/sizeof(menu_t), des);
}

void uart_init(void)
{
    int i;
    pthread_t rx_thread;

    for (i = 0; i < sizeof(uarts)/sizeof(uart_t); i++) {
        if (open_uart(&uarts[i]) < 0) {
            printf("uart_init() fail\n");
            exit(1);
        }
    }

    if (pthread_create(&rx_thread, NULL, (void *)uart_rx_thread, NULL) < 0)
        exit(1);

    printf("UART Init Ok.\n");
}


void lte_tx(char *buf, int len)
{
    uart_ctrl_t txd;

    txd.data = buf;
    txd.len = len;

    if (write_uart(&uarts[UART_LTE], &txd) < 0)
        return;

#ifdef UART_DEBUG
    printf("\n=====[TX.%s.%s]====================\n", uarts[UART_LTE].dev, "UART_LTE");
    uart_hex_print(txd.data, txd.len);
#endif
}

void wvu_tx(char *buf, int len)
{
    uart_ctrl_t txd;

    txd.data = buf;
    txd.len = len;

    if (write_uart(&uarts[UART_WVU], &txd) < 0)
        return;

#ifdef UART_DEBUG
    printf("\n=====[TX.%s.%s]====================\n", uarts[UART_WVU].dev, "UART_WVU");
    uart_hex_print(txd.data, txd.len);
#endif
}

void solar_tx(char *buf, int len)
{
    uart_ctrl_t txd;

    txd.data = buf;
    txd.len = len;

    if (write_uart(&uarts[UART_SOLAR], &txd) < 0)
        return;

#ifdef UART_DEBUG
    printf("\n=====[TX.%s.%s]====================\n", uarts[UART_SOLAR].dev, "UART_SOLAR");
    uart_hex_print(txd.data, txd.len);
#endif
}

void water1_tx(char *buf, int len)
{
    uart_ctrl_t txd;

    txd.data = buf;
    txd.len = len;

    if (write_uart(&uarts[UART_WATER1], &txd) < 0)
        return;

#ifdef UART_DEBUG
    printf("\n=====[TX.%s.%s]====================\n", uarts[UART_WATER1].dev, "UART_WATER1");
    uart_hex_print(txd.data, txd.len);
#endif
}

void water2_tx(char *buf, int len)
{
    uart_ctrl_t txd;

    txd.data = buf;
    txd.len = len;

    if (write_uart(&uarts[UART_WATER2], &txd) < 0)
        return;

#ifdef UART_DEBUG
    printf("\n=====[TX.%s.%s]====================\n", uarts[UART_WATER2].dev, "UART_WATER2");
    uart_hex_print(txd.data, txd.len);
#endif
}

