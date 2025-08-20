#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>
#include "usbctl.h"

int show_usb_info(void)
{
    system("lsusb > /home/root/usb_info.log");
    system("echo \"\" >> /home/root/usb_info.log");
    system("echo \"\" >> /home/root/usb_info.log");
    system("lsusb -t >> /home/root/usb_info.log");
    system("echo \"\" >> /home/root/usb_info.log");
    system("echo \"\" >> /home/root/usb_info.log");
    system("usb-devices >> /home/root/usb_info.log");
    system("nano -v -0 /home/root/usb_info.log");
    keypad(stdscr, TRUE);
    clear();
    refresh();
    return 0;
}