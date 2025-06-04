#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>


int check_val_pass(int value);
int check_val_range(int value, int min, int max);
int check_ip_form(const char *ip);
void eth_up(const char *eth, const char *ip, const char *gw, const char *sub);
void eth_down(const char *eth);
int ping_test(const char *ip);

#endif
