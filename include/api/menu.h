#ifndef MENU_H_
#define MENU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

typedef struct st_menu {
    int    (*func)(void);
    char *func_des;
} menu_t;

typedef struct st_menu_args {
    int     (*func)(void *);
    char *func_des;
    void *args;
} menu_args_t;

static int back(void)
{
    return -1;
}

static int back2(void *)
{
    return -1;
}

//void menu_exec(menu_t *menu, int menu_size, const char *menu_des);
void menu_exec(menu_t *menu, int menu_size, const char *menu_des, WINDOW **pr_win);
//void menu_args_exec(menu_args_t *menu_args, int menu_args_size, const char *menu_des);
void menu_args_exec(menu_args_t *menu_args, int menu_args_size, const char *menu_des, WINDOW **pr_win);
int menu_args_input_exec(char *buf, int size, const char *menu_des);
int menu_exit(void);
void menu_init(void);
//void menu_print(menu_t *menus, int menu_size, const char *menu_des);
//void menu_args_print(menu_args_t *menus, int menu_size, const char *menu_des);
void pr_win(WINDOW *win, const char *fmt, ...);

#endif
