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

#define MAX_GPU_MENU_DEPTH     2
static WINDOW *pr_win_gpu[MAX_GPU_MENU_DEPTH];
static int pr_win_gpu_depth = 0;

static int benchmark_2d_render(void)
{
    endwin();
    system("clear");
    system("glmark2-es2-wayland -b effect2d --run-forever --fullscreen");

    clear();
    refresh();
    return 0;
}

static int benchmark_3d_render(void)
{
    endwin();
    system("clear");
    //system("glmark2-es2-wayland -b effect2d --run-forever --fullscreen");
    //pr_win(pr_win_gpu[pr_win_gpu_depth], "Benchmark 3D Rendering...\nPress ctrl-c to stop rendering.\n\n");
    system("glmark2-es2-wayland -b effect3d --run-forever --fullscreen");

    clear();
    refresh();
    return 0;
}

static menu_t gpu_menus[] = {
    {benchmark_2d_render, "BENCHMARK 2D RENDERING"},
    {benchmark_3d_render, "BENCHMARK 3D RENDERING"},
    {back, "back"},
};

int gpu_ctl(void)
{
    char *des = "GPU TEST MENU";
    menu_exec(gpu_menus, sizeof(gpu_menus) / sizeof(menu_t), des, &pr_win_gpu[pr_win_gpu_depth]);
    return 0;
}