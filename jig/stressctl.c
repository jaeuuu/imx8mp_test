#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>
#include "stressctl.h"

#define MAX_STRESS_MENU_DEPTH     2
static WINDOW *pr_win_stress[MAX_STRESS_MENU_DEPTH];
static int pr_win_stress_depth = 0;

static int stress_exec(const char *cmd)
{
    pr_win(pr_win_stress[pr_win_stress_depth], "Executing command: %s\n\nPress ctrl-c to stop\n", cmd);
    system(cmd);
    pr_win(pr_win_stress[pr_win_stress_depth], "Command execution finished.\n");
    return 0;
}

static int duration = 1;
static int set_test_duration(void)
{
    char des[256];
    char tmp[32];

    snprintf(des, sizeof(des), "Current Duration: [%d min]", duration);

    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), des) < 0)
        return 0;

    if (!atoi(tmp)) {
        duration = 1;
        pr_win(pr_win_stress[pr_win_stress_depth], "[SET TEST DURATION]: invalid value, set to default [%d min]\n", duration);
    } else {
        duration = atoi(tmp);
        pr_win(pr_win_stress[pr_win_stress_depth], "[SET TEST DURATION]: [%d min]\n", duration);
    }
    return 0;
}

static int all_in_one_test(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stress-ng --cpu 0 --cpu-load 100 --cpu-method all --matrix 0 --vm 0 --vm-bytes 95%% --timeout %dm --metrics --tz > /home/root/Logs/stress.log 2>&1", duration);
    return stress_exec(cmd);
}

static int cpu_load_test(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stress-ng --cpu 0 --cpu-load 100 --cpu-method all --timeout %dm --metrics --tz > /home/root/Logs/stress.log 2>&1", duration);
    return stress_exec(cmd);
}

static int memory_load_test(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stress-ng --vm 0 --vm-bytes 95%% --timeout %dm --metrics --tz > /home/root/Logs/stress.log 2>&1", duration);
    return stress_exec(cmd);
}

static int matrix_ops_test(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stress-ng --matrix 0 --timeout %dm --metrics --tz > /home/root/Logs/stress.log 2>&1", duration);
    return stress_exec(cmd);
}

static int show_test_log(void)
{
    //pr_win(pr_win_stress[pr_win_stress_depth], "Showing test log...\n");
    endwin();
    system("clear");
    system("cat /home/root/Logs/stress.log; tail -n 0 -f /home/root/Logs/stress.log");
    //pr_win(pr_win_stress[pr_win_stress_depth], "End of test log.\n");
    keypad(stdscr, TRUE);
    clear();
    refresh();
    return 0;
}

static menu_t stress_menus[] = {
    {set_test_duration, "SET TEST DURATION"},
    {all_in_one_test, "ALL IN ONE TEST"},
    {cpu_load_test, "CPU LOAD TEST"},
    {memory_load_test, "MEMORY LOAD TEST"},
    {matrix_ops_test, "MATRIX OPS TEST"},
    {show_test_log, "SHOW TEST LOG"},
    {back, "back"},
};

int stress_ctl(void)
{
    char *des = "STRESS TEST MENU";
    menu_exec(stress_menus, sizeof(stress_menus) / sizeof(menu_t), des, &pr_win_stress[pr_win_stress_depth]);
    return 0;
}

void stress_init(void)
{
}