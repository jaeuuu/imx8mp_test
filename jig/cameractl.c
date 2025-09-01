#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include "cameractl.h"

#define DEV_PATH_CAMERA_VIDEO "/dev/v4l/by-path/platform-32c00000.bus:camera-video-index0"

#define MAX_CAMERA_MENU_DEPTH     2
static WINDOW *pr_win_camera[MAX_CAMERA_MENU_DEPTH];
static int pr_win_camera_depth = 0;

static int stream_video(void)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gst-launch-1.0 v4l2src device=%s ! videoconvert ! autovideosink > /dev/null 2>&1", DEV_PATH_CAMERA_VIDEO);
    pr_win(pr_win_camera[pr_win_camera_depth], "Video streaming started...\nPress ctrl-c to stop streaming.\n");
    // system("gst-launch-1.0 v4l2src device=/dev/video3 ! videoconvert ! autovideosink > /dev/null 2>&1");
    system(cmd);
    pr_win(pr_win_camera[pr_win_camera_depth], "Video streaming stopped.\n");
    return 0;
}

static int capture_image(void)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gst-launch-1.0 v4l2src device=%s num-buffers=1 ! videoconvert ! pngenc ! filesink location=/home/root/Logs/capturs.png > /dev/null 2>&1", DEV_PATH_CAMERA_VIDEO);
    pr_win(pr_win_camera[pr_win_camera_depth], "Image capturing started...\n");
    //system("gst-launch-1.0 v4l2src device=/dev/video3 num-buffers=1 ! videoconvert ! pngenc ! filesink location=capturs.png > /dev/null 2>&1");
    system(cmd);
    pr_win(pr_win_camera[pr_win_camera_depth], "Image captured. output: capturs.png\n");
    sleep(1);
    pr_win(pr_win_camera[pr_win_camera_depth], "Press ctrl-c to stop showing image.\n");
    system("gst-launch-1.0 filesrc location=/home/root/Logs/capturs.png ! pngdec ! imagefreeze ! videoconvert ! autovideosink > /dev/null 2>&1");
    return 0;
}

static menu_t camera_menus[] = {
    {stream_video, "VIDEO STREAMING"},
    {capture_image, "CAPTURE IMAGE"},
    {back, "back"},
};

int camera_ctl(void)
{
    char *des = "CAMERA TEST MENU";
    menu_exec(camera_menus, sizeof(camera_menus) / sizeof(menu_t), des, &pr_win_camera[pr_win_camera_depth]);
    return 0;
}

void camera_init(void)
{
}