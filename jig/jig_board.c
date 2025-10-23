#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>
#include "gpioctl.h"
#include "uartctl.h"
#include "netctl.h"
#include "canctl.h"
#include "lcdctl.h"
#include "cameractl.h"
#include "stressctl.h"
#include "usbctl.h"
#include "audioctl2.h"
#include "gpuctl.h"
#include "i2cctl.h"

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
    sleep(10);
    return 0;
}

static menu_t main_menu[] = {
    {gpio_input_monitor, "GPIO MONITORING"},
    {gpio_in_ctl, "GPIO INPUT TEST"},
    {gpio_out_ctl, "GPIO OUTPUT TEST"},
    {uart_ctl, "UART TEST"},
    {i2c_ctl, "I2C TEST"},
    {net_ctl, "NETWORK TEST"},
    {can_ctl, "CAN TEST"},
    {lcd_ctl, "LCD TEST"},
    {camera_ctl, "CAMERA TEST"},
    {audio2_ctl, "AUDIO TEST"},
    {stress_ctl, "STRESS TEST"},
    {usb_ctl, "USB TEST"},
    {gpu_ctl, "GPU TEST"},
    {back, "back"},
};

void board_init(int argc, char **argv)
{
    // printf("jig board_init()\r\n");

    if (argc < 2) {
        printf("\nUsage: %s <lcd_type> [term_type]\n\n@example\n\t@1. jigtest lvds\t\t# lvds panel, xterm-256color \
            \n\t@2. jigtest dsi\t\t\t# dsi panel, xterm-256color \
            \n\t@3. jigtest lvds xterm\t\t# lvds panel, xterm-8color \
            \n\t@4. jigtest dsi xterm\t\t# dsi panel, xterm-8color\n\n", argv[0]);
        exit(1);
    }

    lcd_init(argv[1]);
    menu_init(argv[2]);
    gpio_init();
    uart_init();
    can_init();
    i2c_init();
}

void start_board(int argc, char **argv)
{
    // printf("jig start_board()\r\n");
    char *des = "IMX8M-PLUS JIG BOARD TEST";
restart:
    menu_exec(main_menu, sizeof(main_menu) / sizeof(menu_t), des, NULL);

    if (menu_exit() < 0)
        goto restart;
}