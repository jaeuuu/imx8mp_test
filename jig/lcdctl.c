#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include "lcdctl.h"

#define MAX_LCD_MENU_DEPTH     2
static WINDOW *pr_win_lcd[MAX_LCD_MENU_DEPTH];
static int pr_win_lcd_depth = 0;

static int show_triangle(void)
{
    FILE *fp = NULL;
    char buf[512];
    //system("systemctl stop weston\n");
    //system("kmscube\n");
    memset(buf, 0x00, sizeof(buf));
    /*
    fp = popen("weston-simple-egl -f 2>&1", "r");
    sleep(1);
    while (!fread(buf, sizeof(buf), 1, fp)) {
        //fread(buf, sizeof(buf), 1, fp);
        pr_win(pr_win_lcd[pr_win_lcd_depth], "%s", buf);
        memset(buf, 0x00, sizeof(buf));
        usleep(100 * 1000);
    }
    pclose(fp);
    */
    pr_win(pr_win_lcd[pr_win_lcd_depth], "Press ctrl-c to stop showing triangle.\n");
    system("weston-simple-egl -f > /dev/null 2>&1");
    //system("systemctl start weston\n");
    //pr_win(pr_win_lcd[pr_win_lcd_depth], "Press ctrl-c to stop showing triangle.\n");
    return 0;
}

static int show_terminal(void)
{
    pr_win(pr_win_lcd[pr_win_lcd_depth], "Press ctrl-c to stop showing terminal.\n");
    system("weston-terminal -f");
    //pr_win(pr_win_lcd[pr_win_lcd_depth], "Press ctrl-c to stop showing terminal.\n");
    return 0;
}

static int touch_calibration_exec(const char *cmd)
{
    pr_win(pr_win_lcd[pr_win_lcd_depth], "Press ctrl-c to stop showing touch calibration.\n");
    system(cmd);
    return 0;
}

static const char *lcd_type = NULL;
static int touch_calibration(void)
{
    char cmd[256];
    sprintf(cmd, "weston-touch-calibrator %s > /dev/null 2>&1", lcd_type);
    system(cmd);
    // menu_args_t touch_calibration_menu[] = {
    //     {touch_calibration_exec, "LVDS TOUCH", "weston-touch-calibrator LVDS-1 > /dev/null 2>&1"},
    //     {touch_calibration_exec, "MIPI-DSI TOUCH", "weston-touch-calibrator DSI-1 > /dev/null 2>&1"},
    //     {back2, "back", ""}
    // };

    // pr_win_lcd_depth++;
    // menu_args_exec(touch_calibration_menu, sizeof(touch_calibration_menu) / sizeof(menu_args_t), "TOUCH CALIBRATION MENU", &pr_win_lcd[pr_win_lcd_depth]);
    // pr_win_lcd_depth--;
}

static menu_t uart_menus[] = {
    {show_triangle, "SHOW TRIANGLE"},
    {touch_calibration, "TOUCH CALIBRATION"},
    {back, "back"},
};

int lcd_ctl(void)
{
    char *des = "LCD TEST MENU";
    menu_exec(uart_menus, sizeof(uart_menus) / sizeof(menu_t), des, &pr_win_lcd[pr_win_lcd_depth]);
}

void lcd_init(const char *type)
{
    if (!strcmp(type, "dsi")) {
        system("cp /etc/xdg/weston/weston_dsi.ini /etc/xdg/weston/weston.ini");
        lcd_type = "DSI-1";
    } else if (!strcmp(type, "lvds")) {
        system("cp /etc/xdg/weston/weston_lvds.ini /etc/xdg/weston/weston.ini");
        lcd_type = "LVDS-1";
    } else {
        printf("Unknown LCD type: %s\n", type);
        exit(1);
    }

    system("systemctl restart weston");
}