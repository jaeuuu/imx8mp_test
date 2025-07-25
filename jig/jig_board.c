#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>
#include "gpioctl.h"
#include "uartctl.h"
#include "netctl.h"

int hello_world1(void);
int hello_world2(void);
int hello_world3(void);

static menu_t sub_menu[] = {
    {hello_world1, "Sub Hello World 1"},
    {hello_world2, "Sub Hello World 2"},
    {hello_world3, "Sub Hello World 3"},
    {back, "back"},
};

static WINDOW *pr_hello_win;

int sub_hello_world(void)
{
    menu_exec(sub_menu, sizeof(sub_menu) / sizeof(menu_t), "SUB HELLO WORLD MENU", &pr_hello_win);
    return 0;
}

static char buffer[32];
int hello_world1(void)
{
    int x, y;
    char buf[32];
    char des[64];
    getyx(pr_hello_win, y, x);
    mvwprintw(pr_hello_win, y, x, "hello, world1\n");
    wrefresh(pr_hello_win);

    memset(buf, 0x00, sizeof(buf));
    sprintf(des, "Input string [%s]", buffer);

    if (!menu_args_input_exec(buf, sizeof(buf), des))
        memcpy(buffer, buf, sizeof(buf));
    return 0;
}

int hello_world2(void)
{
    int x, y;
    getyx(pr_hello_win, y, x);
    mvwprintw(pr_hello_win, y, x, "hello, world2\n");
    wrefresh(pr_hello_win);
    return 0;
}

int hello_world3(void)
{
    int x, y;
    getyx(pr_hello_win, y, x);
    mvwprintw(pr_hello_win, y, x, "hello, world3\n");
    wrefresh(pr_hello_win);
    return 0;
}

static menu_t main_menu[] = {
    {gpio_in_ctrl, "GPIO INPUT CONTROL"},
    {gpio_out_ctrl, "GPIO OUTPUT CONTROL"},
    {uart_ctrl, "UART CONTROL"},
    {net_ctrl, "NET CONTROL"},
    {hello_world3, "Hello World 3"},
    {sub_hello_world, "Sub Hello World"},
    {back, "back"},
};

void board_init(void)
{
    printf("jig board_init()\r\n");
    menu_init();
    gpio_init();
    uart_init();
}

void start_board(void)
{
    printf("jig start_board()\r\n");
restart:
    menu_exec(main_menu, sizeof(main_menu) / sizeof(menu_t), "IMX8M-PLUS JIG BOARD TEST PROGRAM", NULL);

    if (menu_exit() < 0)
        goto restart;
}