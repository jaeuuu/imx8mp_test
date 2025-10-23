#define _GNU_SOURCE
#include <api/menu.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <errno.h>
#include "i2cctl.h"
#include "ina260.h"

#define MAX_I2C_MENU_DEPTH     3
static WINDOW *pr_win_i2c[MAX_I2C_MENU_DEPTH];
static int pr_win_i2c_depth = 0;

static int get_voltage(void)
{
    float voltage = ina260_get_voltage();
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Voltage: %.3f V\n", voltage);
    return 0;
}

static int get_current(void)
{
    float current = ina260_get_current();
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Current: %.3f A\n", current);
    return 0;
}

static int get_power(void)
{
    float power = ina260_get_power();
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Power: %.3f W\n", power);
    return 0;
}

static int get_manufacturer_id(void)
{
    int id = ina260_get_manufacturer_id();
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Manufacturer ID: 0x%04X\n", id);
    return 0;
}

static int get_die_id(void)
{
    int id = ina260_get_die_id();
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Die ID: 0x%04X\n", id);
    return 0;
}

static int set_curr_limit_over(void)
{
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Function not implemented yet.\n");
    return 0;
}

static int set_curr_limit_under(void)
{
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Function not implemented yet.\n");
    return 0;
}

static int set_volt_limit_over(void)
{
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Function not implemented yet.\n");
    return 0;
}

static int set_volt_limit_under(void)
{
    pr_win(pr_win_i2c[pr_win_i2c_depth], "Function not implemented yet.\n");
    return 0;
}

static menu_t ina260_menus[] = {
    {get_voltage, "GET VOLTAGE"},
    {get_current, "GET CURRENT"},
    {get_power, "GET POWER"},
    {set_curr_limit_over, "SET CURRENT LIMIT OVER"},
    {set_curr_limit_under, "SET CURRENT LIMIT UNDER"},
    {set_volt_limit_over, "SET VOLTAGE LIMIT OVER"},
    {set_volt_limit_under, "SET VOLTAGE LIMIT UNDER"},
    {get_manufacturer_id, "GET MANUFACTURER ID"},
    {get_die_id, "GET DIE ID"},
    {back, "back"},
};

static int ina260_ctl(void)
{
    char *des = "INA260 TEST MENU";
    pr_win_i2c_depth++;
    menu_exec(ina260_menus, sizeof(ina260_menus) / sizeof(menu_t), des, &pr_win_i2c[pr_win_i2c_depth]);
    pr_win_i2c_depth--;
    return 0;
}

static menu_t i2c_menus[] = {
    {ina260_ctl, "INA260 TEST"},
    {back, "back"},
};

int i2c_ctl(void)
{
    char *des = "I2C TEST MENU";
    menu_exec(i2c_menus, sizeof(i2c_menus) / sizeof(menu_t), des, &pr_win_i2c[pr_win_i2c_depth]);
    return 0;
}

void i2c_init(void)
{
    ina260_init();
}