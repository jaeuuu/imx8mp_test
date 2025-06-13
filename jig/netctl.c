#include "netctl.h"

/*
 * MENU Deepth 4
 */

static void eth_static_set_ip(void)
{
    int i, num;
    char c, ip[32];

    while (1) {
        memset(ip, 0x00, sizeof(ip));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [xxx.xxx.xxx.xxx]\n\n");
        printf("\t   (ex) >> 192.168.25.100\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(ip))
                ip[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(ip, i);
        if (num == 0)
            return;

        if (check_ip_form(ip) < 0) {
            printf("invalid ip format.\n");
            continue;
        }
        eth_up("eth0", ip, NULL, NULL);
    }
}

static void eth_static_set_gw(void)
{
    int i, num;
    char c, gw[32];

    while (1) {
        memset(gw, 0x00, sizeof(gw));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [xxx.xxx.xxx.xxx]\n\n");
        printf("\t   (ex) >> 192.168.25.1\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(gw))
                gw[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(gw, i);
        if (num == 0)
            return;

        if (check_ip_form(gw) < 0) {
            printf("invalid ip format.\n");
            continue;
        }
        eth_up("eth0", NULL, gw, NULL);
    }
}

static void eth_static_set_sub(void)
{
    int i;
    long long num;
    char c, sub[32];

    while (1) {
        memset(sub, 0x00, sizeof(sub));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [xxx.xxx.xxx.xxx]\n\n");
        printf("\t   (ex) >> 255.255.255.0\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(sub))
                sub[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(sub, i);
        if (num == 0)
            return;

        if (check_ip_form(sub) < 0) {
            printf("invalid ip format.\n");
            continue;
        }
        eth_up("eth0", NULL, NULL, sub);
    }
}

static menu_t eth_static_menus[] = {
    {eth_static_set_ip, "IP SETTING"},
    {eth_static_set_gw, "GATEWAY SETTING"},
    {eth_static_set_sub, "SUBNET SETTING"},
};

/*
 * MENU Deepth 3
 */

static void eth_static(void)
{
    char *des = "\t  Ethernet Static Setting Menu";
    menu_print(eth_static_menus, sizeof(eth_static_menus)/sizeof(menu_t), des);
}

static void eth_dhcp(void)
{
    if (system("udhcpc -n -q -i eth0 &") < 0)
        printf("udhcpc -n -q -i eth0 fail!\n");
}

static menu_t net_eth_enable_menus[] = {
    {eth_static, "STATIC"},
    {eth_dhcp, "DHCP"},
};

/*
 * MENU Deepth 2
 */

static void eth_enable(void)
{
    char *des = "\t  Ehternet Enabling Mode Menu";
    menu_print(net_eth_enable_menus, sizeof(net_eth_enable_menus)/sizeof(menu_t), des);
}

static void eth_disable(void)
{
    eth_down("eth0");
}

static void wwan_enable(void)
{
    wwan_up("wwan0");
}

static void wwan_disable(void)
{
    wwan_down("wwan0");
}

static void eth_ping(void)
{
    int i;
    long long num;
    char c, ip[32];

    while (1) {
        memset(ip, 0x00, sizeof(ip));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [xxx.xxx.xxx.xxx]\n\n");
        printf("\t   (ex) >> 8.8.8.8\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(ip))
                ip[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(ip, i);
        if (num == 0)
            return;

        if (check_ip_form(ip) < 0) {
            printf("invalid ip format.\n");
            continue;
        }
        ping_test("eth0", ip);
    }
}

static void wwan_ping(void)
{
    int i;
    long long num;
    char c, ip[32];

    while (1) {
        memset(ip, 0x00, sizeof(ip));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [xxx.xxx.xxx.xxx]\n\n");
        printf("\t   (ex) >> 8.8.8.8\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(ip))
                ip[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail!\n");

        num = check_ascii_num(ip, i);
        if (num == 0)
            return;

        if (check_ip_form(ip) < 0) {
            printf("invalid ip format.\n");
            continue;
        }
        ping_test("wwan0", ip);
    }
}

static menu_t net_eth_menus[] = {
    {eth_enable, "ENABLE"},
    {eth_disable, "DISABLE"},
};

static menu_t net_wwan_menus[] = {
    {wwan_enable, "ENABLE"},
    {wwan_disable, "DISABLE"},
};

static menu_t net_pingtest_menus[] = {
    {eth_ping, "ETHERNET PING TEST"},
    {wwan_ping, "WIRELESS WAN PING TEST"},
};

/*
 * MENU Deepth 1
 */

static void net_eth(void)
{
    char *des = "\t     Ethernet Control Menu";
    menu_print(net_eth_menus, sizeof(net_eth_menus)/sizeof(menu_t), des);
}

static void net_wwan(void)
{
    char *des = "\t   Wireless WAN Control Menu";
    menu_print(net_wwan_menus, sizeof(net_wwan_menus)/sizeof(menu_t), des);
}

static void net_pingtest(void)
{
    char *des = "\t        Ping Test Menu";
    menu_print(net_pingtest_menus, sizeof(net_pingtest_menus)/sizeof(menu_t), des);
}

static void net_info(void)
{
    if (system("ifconfig") < 0)
        printf("ifconfig fail!\n");
    printf("\n\n");
    if (system("route -n") < 0)
        printf("route -n fail\n");
}

static menu_t net_menus[] = {
    {net_eth, "EHTERNET"},
    {net_wwan, "WIRELESS WAN"},
    {net_info, "NETWORK INFO"},
    {net_pingtest, "PING TEST"},
};

void net_control(void)
{
    char *des = "\t      Network Control Menu";
    menu_print(net_menus, sizeof(net_menus)/sizeof(menu_t), des);
}
