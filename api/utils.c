#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <regex.h>
#include <api/debug.h>

int check_val_pass(int value)
{
    return 0;
}

int check_val_range(int value, int min, int max)
{
    if (value < min || value > max)
        return -1;
    return 0;
}

int check_ip_form(const char *ip)
{
    regex_t regex;
    //int c = regcomp(&regex, "^\\(25[0-5]\\|2[0-4][0-9]\\|[0-1]?[0-9][0-9]?\\)\\(\\.\\(25[0-5]\\|2[0-4][0-9]\\|[0-1]?[0-9][0-9]?\\)\\)\\{3\\}$",
     //   REG_EXTENDED);

    //int c = regcomp(&regex, "^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$", REG_EXTENDED);
    //int c = regcomp(&regex, "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$", REG_EXTENDED);
    int c = regcomp(&regex, "^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])$", REG_EXTENDED);

    if (c) {
        perror("regcomp() fail");
        return -1;
    }

    c = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);

    if (!c) {
        dlp_dbg("a valid ip format");
        return 0;
    } else {
        dlp_dbg("not a valid ip format");
        return -1;
    }
}

void eth_up(const char *eth, const char *ip, const char *gw, const char *sub)
{
    char cmd[256];

    dlp_dbg("|eth : %s\r\n|ip : %s\r\n|gw : %s\r\n|sub : %s", eth, ip, gw, sub);

    if (eth) {
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd, "ifconfig %s up", eth);
        system(cmd);

        if (ip) {
            memset(cmd, 0x00, sizeof(cmd));
            sprintf(cmd, "ifconfig %s %s", eth, ip);
            system(cmd);
        }

        if (gw) {
            memset(cmd, 0x00, sizeof(cmd));
            sprintf(cmd, "route add default gw %s dev %s", gw, eth);
            system(cmd);
        }

        if (sub) {
            memset(cmd, 0x00, sizeof(cmd));
            sprintf(cmd, "ifconfig %s netmask %s", eth, sub);
            system(cmd);
        }
    }
}

void eth_down(const char *eth)
{
    char cmd[256];

    dlp_dbg("|eth : %s", eth);

    if (eth) {
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd, "ifconfig %s down", eth);
        system(cmd);
    }
}

int check_ip_match(unsigned long ip1, unsigned long ip2)
{
    if (ip1 == ip2)
        return 0;
    else
        return -1;
}

unsigned long ip_to_ulong_le(char *ip)
{
    struct in_addr addr;

    if (inet_aton(ip, &addr) > 0)
        return ntohl(addr.s_addr);
    else
        return -1;
}

unsigned long ip_to_ulong_be(char *ip)
{
    struct in_addr addr;

    if (inet_aton(ip, &addr) > 0)
        return addr.s_addr;
    else
        return -1;
}

char *ip_to_str(unsigned long ip)
{
    struct in_addr addr;

    addr.s_addr = htonl(ip);

    return inet_ntoa(addr);
}

int ping_test(const char *ip)
{
    char tmp[64];

    if (check_ip_form(ip) < 0)
        return -1;

    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp, "ping -c 5 %s", ip);

    if (!system(tmp))
        return 0;
    else
        return -1;
}
