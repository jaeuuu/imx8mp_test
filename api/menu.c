#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>
#include <stdarg.h>

/* In my environment, max width 158, max height 60 */

#define CONSOLE_TOTAL_W     120
#define CONSOLE_TOTAL_H     60

#define MAX_MENU_TITLE_W    50
#define MAX_MENU_TITLE_H    5

#define MAIN_MENU_START_X   0

static void menu_print(menu_t *menus, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    int i;
    int x, y;
    int h, w;
    int pos = 0;
    WINDOW *menu_title;
    WINDOW *menu_win;
    WINDOW *menu_sub_win;
    WINDOW *result_win;

    if (menu_size <= 0) {
        printf("menu_size error! [menu_size: %d]\n", menu_size);
        clear();
        refresh();
        endwin();
        exit(1);
    }

    menu_title = newwin(MAX_MENU_TITLE_H, MAX_MENU_TITLE_W, 0, 0);
    menu_win = newwin(menu_size + 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);     // +1 margin
    menu_sub_win = subwin(menu_win, menu_size, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + 1, MAIN_MENU_START_X + 1);
    //box(menu_win, 0, 0);

    /* result box */
    if (pr_win) {
        //result_title = newwin(7, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);
        result_win = newwin(CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H + menu_size + 2, MAIN_MENU_START_X);
        *pr_win = subwin(result_win, CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2 - 2, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + menu_size + 2 + 1, MAIN_MENU_START_X + 1);
        //scrollok(result_win, TRUE);
        scrollok(*pr_win, TRUE);
    }

    //box(result_title, 0, 0);
    box(menu_title, 0, 0);
    box(menu_win, 0, 0);
    if (pr_win)
        box(result_win, 0, 0);

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            mvwprintw(menu_title, y, x, "%s", menu_des);
        }
        touchwin(menu_title);
        wrefresh(menu_title);

        touchwin(menu_win);
        wrefresh(menu_win);

        if (pr_win) {
            touchwin(result_win);
            wrefresh(result_win);
        }

        //getyx(menu_win, y, x);

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                wattron(menu_sub_win, A_REVERSE);
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                wattroff(menu_sub_win, A_REVERSE);
            } else
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_UP:
            pos = (pos - 1 + menu_size) % menu_size;
            break;
        case KEY_DOWN:
            pos = (pos + 1 + menu_size) % menu_size;
            break;
        case 10:    // enter
            if ((menus + pos)->func() < 0) {
                clear();
                refresh();
                delwin(menu_title);
                delwin(menu_sub_win);
                delwin(menu_win);
                //werase(menu_title);
                //werase(menu_sub_win);
                //werase(menu_win);
                if (pr_win) {
                    delwin(*pr_win);
                    delwin(result_win);
                    //werase(*pr_win);
                    //werase(result_win);
                    *pr_win = NULL;
                }
                endwin();
                return;
            }
            break;
        case 27:    // esc
            clear();
            refresh();
            delwin(menu_title);
            delwin(menu_sub_win);
            delwin(menu_win);
            if (pr_win) {
                delwin(*pr_win);
                delwin(result_win);
                *pr_win = NULL;
            }
            endwin();
            exit(0);
        }
    }
}

static void menu_args_print(menu_args_t *menus, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    int i;
    int x, y;
    int h, w;
    int pos = 0;
    WINDOW *menu_title;
    WINDOW *menu_win;
    WINDOW *menu_sub_win;
    WINDOW *result_win;

    if (menu_size <= 0) {
        printf("menu_size error! [menu_size: %d]\n", menu_size);
        clear();
        refresh();
        endwin();
        exit(1);
    }

    menu_title = newwin(MAX_MENU_TITLE_H, MAX_MENU_TITLE_W, 0, 0);
    menu_win = newwin(menu_size + 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);     // +1 margin
    menu_sub_win = subwin(menu_win, menu_size, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + 1, MAIN_MENU_START_X + 1);

    if (pr_win) {
        result_win = newwin(CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H + menu_size + 2, MAIN_MENU_START_X);
        *pr_win = subwin(result_win, CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2 - 2, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + menu_size + 2 + 1, MAIN_MENU_START_X + 1);
        scrollok(*pr_win, TRUE);
    }

    box(menu_title, 0, 0);
    box(menu_win, 0, 0);
    if (pr_win)
        box(result_win, 0, 0);

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            mvwprintw(menu_title, y, x, "%s", menu_des);
        }

        touchwin(menu_title);
        wrefresh(menu_title);

        touchwin(menu_win);
        wrefresh(menu_win);

        if (pr_win) {
            touchwin(result_win);
            wrefresh(result_win);
        }

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                wattron(menu_sub_win, A_REVERSE);
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                wattroff(menu_sub_win, A_REVERSE);
            } else
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_UP:
            pos = (pos - 1 + menu_size) % menu_size;
            break;
        case KEY_DOWN:
            pos = (pos + 1 + menu_size) % menu_size;
            break;
        case 10:    // enter
            if ((menus + pos)->func((menus + pos)->args) < 0) {
                clear();
                refresh();
                delwin(menu_title);
                delwin(menu_sub_win);
                delwin(menu_win);
                //werase(menu_title);
                //werase(menu_sub_win);
                //werase(menu_win);
                if (pr_win) {
                    delwin(*pr_win);
                    delwin(result_win);
                    //werase(*pr_win);
                    //werase(result_win);
                    *pr_win = NULL;
                }
                endwin();
                return;
            }
            break;
        case 27:    // esc
            clear();
            refresh();
            delwin(menu_title);
            delwin(menu_sub_win);
            delwin(menu_win);
            if (pr_win) {
                delwin(*pr_win);
                delwin(result_win);
                *pr_win = NULL;
            }
            endwin();
            exit(0);
        }
    }
}

void menu_exec(menu_t *menu, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    clear();
    refresh();
    menu_print(menu, menu_size, menu_des, pr_win);
}

void menu_args_exec(menu_args_t *menu_args, int menu_args_size, const char *menu_des, WINDOW **pr_win)
{
    clear();
    refresh();
    menu_args_print(menu_args, menu_args_size, menu_des, pr_win);
}

void menu_init(void)
{
    system("dmesg -n 1");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

static int is_yes(void)
{
    return 0;
}

static int is_no(void)
{
    return -1;
}

static menu_t exit_menu[] = {
    {is_yes, "Yes"},
    {is_no, "No"},
};

int menu_exit(void)
{
    char *des = "Are you sure you want to quit?";
    int i;
    int x, y, h, w;
    int pos = 0;
    int menu_size = sizeof(exit_menu) / sizeof(menu_t);
    WINDOW *menu_title;
    WINDOW *menu_win;
    WINDOW *menu_sub_win;

    menu_title = newwin(MAX_MENU_TITLE_H, MAX_MENU_TITLE_W, 0, 0);
    menu_win = newwin(menu_size + 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);     // +1 margin
    menu_sub_win = subwin(menu_win, menu_size, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + 1, MAIN_MENU_START_X + 1);

    box(menu_title, 0, 0);
    box(menu_win, 0, 0);

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (des) {
            x -= strlen(des) / 2;
            mvwprintw(menu_title, y, x, "%s", des);
        }

        touchwin(menu_title);
        wrefresh(menu_title);

        touchwin(menu_win);
        wrefresh(menu_win);

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                wattron(menu_sub_win, A_REVERSE);
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
                wattroff(menu_sub_win, A_REVERSE);
            } else
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_UP:
            pos = (pos - 1 + menu_size) % menu_size;
            break;
        case KEY_DOWN:
            pos = (pos + 1 + menu_size) % menu_size;
            break;
        case 10:    // enter
            clear();
            refresh();
            delwin(menu_title);
            delwin(menu_sub_win);
            delwin(menu_win);
            //werase(menu_title);
            //werase(menu_sub_win);
            //werase(menu_win);
            endwin();
            if ((exit_menu + pos)->func() < 0)
                return -1;
            return 0;
        case 27:    // esc
            clear();
            refresh();
            delwin(menu_title);
            delwin(menu_sub_win);
            delwin(menu_win);
            endwin();
            exit(0);
        }
    }
}

void pr_win(WINDOW *win, const char *fmt, ...)
{
    char tmp[256];
    va_list args;
    int x, y;

    if (!win)
        return;

    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    getyx(win, y, x);
    mvwprintw(win, y, x, "%s", tmp);
    wrefresh(win);
}