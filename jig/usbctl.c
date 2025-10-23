#define _GNU_SOURCE
#include <api/menu.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <libudev.h>
#include <libmount/libmount.h>
#include <errno.h>
#include "usbctl.h"

#define MAX_USB_MENU_DEPTH     2
static WINDOW *pr_win_usb[MAX_USB_MENU_DEPTH];
static int pr_win_usb_depth = 0;

static int show_usb_info(void)
{
#if 0
    system("lsusb > /home/root/Logs/usb_info.log");
    system("echo \"\" >> /home/root/Logs/usb_info.log");
    system("echo \"\" >> /home/root/Logs/usb_info.log");
    system("lsusb -t >> /home/root/Logs/usb_info.log");
    system("echo \"\" >> /home/root/Logs/usb_info.log");
    system("echo \"\" >> /home/root/Logs/usb_info.log");
    system("usb-devices >> /home/root/Logs/usb_info.log");
    system("nano -v -0 /home/root/Logs/usb_info.log");
#endif
    endwin();
    system("clear");
    system("dmesg -w");
    keypad(stdscr, TRUE);
    clear();
    refresh();
    return 0;
}

static int create_file_to_usb_flash(const char *path)
{
    char cmd[256];

    pr_win(pr_win_usb[pr_win_usb_depth], "USB Flash Test: %s\n", path);
    pr_win(pr_win_usb[pr_win_usb_depth], "Create file \"testfile.txt\": %s/testfile.txt\n", path);
    sprintf(cmd, "echo \"This is a test file.\" > \"%s/testfile.txt\"", path);
    if (!system(cmd))
        pr_win(pr_win_usb[pr_win_usb_depth], "File created successfully.\n\n");
    else
        pr_win(pr_win_usb[pr_win_usb_depth], "Failed to create file.\n\n");
    //pr_win(pr_win_usb[pr_win_usb_depth], "File created successfully.\n\n");
    return 0;
}

static int usb_flash_test(void)
{
    struct udev *udev = udev_new();

    if (!udev) {
        pr_win(pr_win_usb[pr_win_usb_depth], "Failed to create udev context.\n");
        return 0;
    }

    struct udev_enumerate *e = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(e, "block");
    udev_enumerate_add_match_property(e, "ID_BUS", "usb");
    udev_enumerate_add_match_property(e, "ID_USB_DRIVER", "usb-storage");
    udev_enumerate_scan_devices(e);

    struct libmnt_table *tb = mnt_new_table();
    if (!tb) {
        pr_win(pr_win_usb[pr_win_usb_depth], "Failed to create libmount table.\n");
        udev_enumerate_unref(e);
        udev_unref(udev);
        return 0;
    }

    if (mnt_table_parse_file(tb, "/proc/mounts") < 0) {
        pr_win(pr_win_usb[pr_win_usb_depth], "Failed to parse /proc/mounts.\n");
        mnt_free_table(tb);
        udev_enumerate_unref(e);
        udev_unref(udev);
        return 0;
    }

    int idx_menu = 0, nr_menu = 1;
    menu_args_t *usb_flash_menu = NULL;
    usb_flash_menu = malloc((sizeof(menu_args_t) * nr_menu));
    if (!usb_flash_menu) {
        pr_win(pr_win_usb[pr_win_usb_depth], "Failed to allocate memory for USB flash menu.\n");
        mnt_free_table(tb);
        udev_enumerate_unref(e);
        udev_unref(udev);
        return 0;
    }

    struct udev_list_entry *dev, *devs = udev_enumerate_get_list_entry(e);
    udev_list_entry_foreach(dev, devs) {
        const char *syspath = udev_list_entry_get_name(dev);
        struct udev_device *d = udev_device_new_from_syspath(udev, syspath);
        if (d) {
            const char *n = udev_device_get_devnode(d);
            if (n) {
                struct libmnt_fs *fs = mnt_table_find_source(tb, n, MNT_ITER_BACKWARD);
                if (fs) {
                    //printf("USB Device: %s\n", n);
                    //printf("Mount Point: %s\n", mnt_fs_get_target(fs));
                    usb_flash_menu[idx_menu].func = create_file_to_usb_flash;
                    usb_flash_menu[idx_menu].func_des = mnt_fs_get_target(fs);
                    usb_flash_menu[idx_menu].args = mnt_fs_get_target(fs);
                    idx_menu++;

                    if (idx_menu >= nr_menu) {
                        nr_menu++;
                        usb_flash_menu = realloc(usb_flash_menu, sizeof(menu_args_t) * nr_menu);
                    }
                }
            }
            udev_device_unref(d);
        }
    }
    usb_flash_menu[idx_menu].func = back2;
    usb_flash_menu[idx_menu].func_des = "back";
    usb_flash_menu[idx_menu].args = NULL;

    pr_win_usb_depth++;
    menu_args_exec(usb_flash_menu, nr_menu, "USB FLASH TEST MENU", &pr_win_usb[pr_win_usb_depth]);
    pr_win_usb_depth--;

    free(usb_flash_menu);
    mnt_free_table(tb);
    udev_enumerate_unref(e);
    udev_unref(udev);
    return 0;
}

static menu_t usb_menus[] = {
    {show_usb_info, "SHOW USB INFO"},
    {usb_flash_test, "USB FLASH TEST"},
    {back, "back"},
};

int usb_ctl(void)
{
    char *des = "USB TEST MENU";
    menu_exec(usb_menus, sizeof(usb_menus) / sizeof(menu_t), des, &pr_win_usb[pr_win_usb_depth]);
    return 0;
}