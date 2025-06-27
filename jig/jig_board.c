#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>
#include "gpioctl.h"
#include "uartctl.h"

int hello_world1(void);
int hello_world2(void);
int hello_world3(void);

static menu_t sub_menu[] = {
    {hello_world1, "Sub Hello World 1"},
    {hello_world2, "Sub Hello World 2"},
    {hello_world3, "Sub Hello World 3"},
    {back, "back"},
};

int sub_hello_world(void)
{
    menu_exec(sub_menu, sizeof(sub_menu) / sizeof(menu_t), "\tSUB HELLO WORLD MENU");
    return 0;
}

int hello_world1(void)
{
    int x, y;
    getyx(stdscr, y, x);
    mvprintw(y, 0, "hello, world1\n");
    refresh();
    return 0;
}

int hello_world2(void)
{
    int x, y;
    getyx(stdscr, y, x);
    mvprintw(y, 0, "hello, world2\n");
    refresh();
    return 0;
}

int hello_world3(void)
{
    int x, y;
    getyx(stdscr, y, x);
    mvprintw(y, 0, "hello, world3\n");
    refresh();
    return 0;
}

static menu_t main_menu[] = {
    {gpio_in_ctrl, "GPIO INPUT CONTROL"},
    {gpio_out_ctrl, "GPIO OUTPUT CONTROL"},
    {uart_ctrl, "UART CONTROL"},
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
    menu_exec(main_menu, sizeof(main_menu) / sizeof(menu_t), "\tIMX8M-PLUS JIG BOARD TEST PROGRAM");

    if (menu_exit() < 0)
        goto restart;
}