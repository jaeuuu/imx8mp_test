#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <api/utils.h>
#include <api/menu.h>
#include "netctl.h"

struct network_info eth0_info;
struct network_info eth1_info;

#define MAX_NET_MENU_DEPTH     2
static WINDOW *pr_win_net[MAX_NET_MENU_DEPTH];
static int pr_win_net_depth = 0;
/*
 * MENU Deepth 4
 */

static int setup_ip(void *intf)
{
    char des[64];
    char tmp[32];
    struct network_info *info;

    if (!strcmp(intf, "eth0"))
        info = &eth0_info;
    else if (!strcmp(intf, "eth1"))
        info = &eth1_info;

    snprintf(des, sizeof(des), "Current IP: [%s]", info->ip);

    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), des) < 0)
        return 0;

    if (check_ip_form(tmp) < 0) {
        wattron(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        pr_win(pr_win_net[pr_win_net_depth], "[SET IP]: format error!\n");
        wattroff(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        return 0;
    }

    strcpy(info->ip, tmp);
    eth_up(intf, info->ip, NULL, NULL);
    pr_win(pr_win_net[pr_win_net_depth], "[SET IP]: [%s]\n", info->ip);
    return 0;
}

static int setup_gw(void *intf)
{
    char des[64];
    char tmp[32];
    struct network_info *info;

    if (!strcmp(intf, "eth0"))
        info = &eth0_info;
    else if (!strcmp(intf, "eth1"))
        info = &eth1_info;

    snprintf(des, sizeof(des), "Current GW: [%s]", info->gw);

    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), des) < 0)
        return 0;

    if (check_ip_form(tmp) < 0) {
        wattron(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        pr_win(pr_win_net[pr_win_net_depth], "[SET GATEWAY]: format error!\n");
        wattroff(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        return 0;
    }

    strcpy(info->gw, tmp);
    eth_up(intf, NULL, info->gw, NULL);
    pr_win(pr_win_net[pr_win_net_depth], "[SET GATEWAY]: [%s]\n", info->gw);
    return 0;
}

static int setup_sub(void *intf)
{
    char des[64];
    char tmp[32];
    struct network_info *info;

    if (!strcmp(intf, "eth0"))
        info = &eth0_info;
    else if (!strcmp(intf, "eth1"))
        info = &eth1_info;

    snprintf(des, sizeof(des), "Current SUB: [%s]", info->sub);

    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), des) < 0)
        return 0;

    if (check_ip_form(tmp) < 0) {
        wattron(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        pr_win(pr_win_net[pr_win_net_depth], "[SET SUBNET]: format error!\n");
        wattroff(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        return 0;
    }

    strcpy(info->sub, tmp);
    eth_up(intf, NULL, info->gw, info->sub);
    pr_win(pr_win_net[pr_win_net_depth], "[SET SUBNET]: [%s]\n", info->sub);
    return 0;
}

static int setup_ping(void *intf)
{
    char cmd[64];
    char tmp[32];
    char buf[512];
    struct network_info *info;
    FILE *fp;

    if (!strcmp(intf, "eth0"))
        info = &eth0_info;
    else if (!strcmp(intf, "eth1"))
        info = &eth1_info;

    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), "Input target IP") < 0)
        return 0;

    if (check_ip_form(tmp) < 0) {
        wattron(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        pr_win(pr_win_net[pr_win_net_depth], "[PING TEST]: format error!\n");
        wattroff(pr_win_net[pr_win_net_depth], COLOR_PAIR(1));
        return 0;
    }

    //sprintf(cmd, "ping -c 5 %s -I %s > /home/root/Logs/ping.log 2>&1 &", tmp, (char *)intf);
    sprintf(cmd, "ping %s -I %s > /home/root/Logs/ping.log 2>&1 &", tmp, (char *)intf);
    //system(cmd);
    endwin();
    system("clear");
    system(cmd);
    system("tail -f /home/root/Logs/ping.log");
    system("killall ping > /dev/null 2>&1");
    // fp = popen(cmd, "r");
    // if (!fp)
    //     return 0;
    // //pr_win(pr_win_net[pr_win_net_depth], "%s\n", cmd);

    // memset(buf, 0x00, sizeof(buf));
    // fread(buf, sizeof(buf), 1, fp);
    // pr_win(pr_win_net[pr_win_net_depth], "%s\n%s\n", cmd, buf);
    // pclose(fp);
    return 0;
}

/*
 * MENU Deepth 1
 */

static int net_eth0(void)
{
    char *des = "ETHERNET1 TEST MENU";
    menu_args_t eth_set_menu[] = {
        {setup_ip, "SET IP", "eth0"},
        {setup_gw, "SET GATEWAY", "eth0"},
        {setup_sub, "SET SUBNET", "eth0"},
        {setup_ping, "PING TEST", "eth0"},
        {back2, "back", "eth0"}
    };

    pr_win_net_depth++;
    menu_args_exec(eth_set_menu, sizeof(eth_set_menu) / sizeof(menu_args_t), des, &pr_win_net[pr_win_net_depth]);
    pr_win_net_depth--;

    return 0;
}

static int net_eth1(void)
{
    char *des = "ETHERNET2 TEST MENU";
    menu_args_t eth_set_menu[] = {
        {setup_ip, "SET IP", "eth1"},
        {setup_gw, "SET GATEWAY", "eth1"},
        {setup_sub, "SET SUBNET", "eth1"},
        {setup_ping, "PING TEST", "eth1"},
        {back2, "back", "eth0"}
    };

    pr_win_net_depth++;
    menu_args_exec(eth_set_menu, sizeof(eth_set_menu) / sizeof(menu_args_t), des, &pr_win_net[pr_win_net_depth]);
    pr_win_net_depth--;

    return 0;
}

static int net_info(void)
{
    FILE *fp;
    char buf[1024];

    // fp = popen("ifconfig", "r");
    // if (!fp)
    //     return 0;

    // memset(buf, 0x00, sizeof(buf));
    // fread(buf, sizeof(buf), 1, fp);

    // pr_win(pr_win_net[pr_win_net_depth], "%s\n", buf);
    // pclose(fp);

    system("ifconfig > /home/root/Logs/net_info.log 2>&1");
    system("echo \"\" >> /home/root/Logs/net_info.log 2>&1");
    system("echo \"\" >> /home/root/Logs/net_info.log 2>&1");
    system("route -n >> /home/root/Logs/net_info.log 2>&1");

    endwin();
    system("clear");
    system("nano -v -0 /home/root/Logs/net_info.log");

    return 0;
}

static menu_t net_menus[] = {
    {net_eth0, "ETHERNET1"},
    {net_eth1, "ETHERNET2"},
    {net_info, "NETWORK INFO"},
    {back, "back"}
};

int net_ctl(void)
{
    char *des = "NETWORK TEST MENU";
    menu_exec(net_menus, sizeof(net_menus) / sizeof(menu_t), des, &pr_win_net[pr_win_net_depth]);

    return 0;
}
