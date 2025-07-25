#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>
#include <stdarg.h>
#include <semaphore.h>

/* In my environment, max width 158, max height 60 */

#define CONSOLE_TOTAL_W     120
#define CONSOLE_TOTAL_H     50

#define MAX_MENU_TITLE_W    50
#define MAX_MENU_TITLE_H    5

#define MAIN_MENU_START_X   0

static void menu_print(menu_t *menus, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    int i;
    int x, y;
    int h, w;
    int pos = 0;
    WINDOW *menu_title = NULL;
    WINDOW *menu_win = NULL;
    WINDOW *menu_sub_win = NULL;
    WINDOW *result_win = NULL;
    MEVENT event;

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

    /* result box */
    if (pr_win) {
        //result_title = newwin(7, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);
        result_win = newwin(CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H + menu_size + 2, MAIN_MENU_START_X);
        *pr_win = subwin(result_win, CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2 - 2, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + menu_size + 2 + 1, MAIN_MENU_START_X + 1);
        //scrollok(result_win, TRUE);
        scrollok(*pr_win, TRUE);
    }

    box(menu_title, 0, 0);
    wbkgd(menu_title, COLOR_PAIR(2));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(2));
    wbkgd(menu_sub_win, COLOR_PAIR(2));
    if (pr_win) {
        box(result_win, 0, 0);
        wbkgd(result_win, COLOR_PAIR(2));
        wbkgd(*pr_win, COLOR_PAIR(2));
    }

#if 1
    h = getmaxy(menu_title);
    w = getmaxx(menu_title);
    wattron(menu_title, COLOR_PAIR(4));
    mvwvline(menu_title, 0, 0, ACS_VLINE, h);
    mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
    mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
    mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
    wattroff(menu_title, COLOR_PAIR(4));
    h = getmaxy(menu_win);
    w = getmaxx(menu_win);
    wattron(menu_win, COLOR_PAIR(4));
    mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
    mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
    wattroff(menu_win, COLOR_PAIR(4));
    h = getmaxy(result_win);
    w = getmaxx(result_win);
    wattron(result_win, COLOR_PAIR(4));
    mvwvline(result_win, 0, w - 1, ACS_VLINE, h);
    mvwhline(result_win, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(result_win, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(result_win, 0, w - 1, ACS_URCORNER);
    wattroff(result_win, COLOR_PAIR(4));
#endif

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(menu_title, COLOR_PAIR(5));
            mvwprintw(menu_title, y, x, "%s", menu_des);
            wattroff(menu_title, COLOR_PAIR(5));
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
                /* old */
                // wattron(menu_sub_win, COLOR_PAIR(3));
                // mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                // wattroff(menu_sub_win, COLOR_PAIR(3));
                /* new */
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, 0, " %2d: ", i + 1);
                wattroff(menu_sub_win, COLOR_PAIR(3));
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(6));
                getyx(menu_sub_win, y, x);
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
                wattroff(menu_sub_win, COLOR_PAIR(3));
            } else {
                /* old */
                //mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                //getyx(menu_sub_win, y, x);
                /* new */
                mvwprintw(menu_sub_win, i, 0, " %2d: ", i + 1);
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(5));
                getyx(menu_sub_win, y, x);
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
            }
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_F(5):  // F5 key
            werase(*pr_win);
            clear();
            refresh();
            break;
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
#if 0
        case KEY_MOUSE:
            getmouse(&event);
            getyx(*pr_win, y, x);
            if (event.bstate & BUTTON4_PRESSED)
                mvwprintw(*pr_win, y, x, "[mouse event][x=%d, y=%d, scroll up]", event.x, event.y);
            else
                mvwprintw(*pr_win, y, x, "[mouse event][x=%d, y=%d, scroll down]", event.x, event.y);
            wrefresh(*pr_win);
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
#endif
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
    wbkgd(menu_title, COLOR_PAIR(2));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(2));
    wbkgd(menu_sub_win, COLOR_PAIR(2));
    if (pr_win) {
        box(result_win, 0, 0);
        wbkgd(result_win, COLOR_PAIR(2));
        wbkgd(*pr_win, COLOR_PAIR(2));
    }

    h = getmaxy(menu_title);
    w = getmaxx(menu_title);
    wattron(menu_title, COLOR_PAIR(4));
    mvwvline(menu_title, 0, 0, ACS_VLINE, h);
    mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
    mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
    mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
    wattroff(menu_title, COLOR_PAIR(4));
    h = getmaxy(menu_win);
    w = getmaxx(menu_win);
    wattron(menu_win, COLOR_PAIR(4));
    mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
    mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
    wattroff(menu_win, COLOR_PAIR(4));
    h = getmaxy(result_win);
    w = getmaxx(result_win);
    wattron(result_win, COLOR_PAIR(4));
    mvwvline(result_win, 0, w - 1, ACS_VLINE, h);
    mvwhline(result_win, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(result_win, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(result_win, 0, w - 1, ACS_URCORNER);
    wattroff(result_win, COLOR_PAIR(4));

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(menu_title, COLOR_PAIR(5));
            mvwprintw(menu_title, y, x, "%s", menu_des);
            wattroff(menu_title, COLOR_PAIR(5));
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
                // wattron(menu_sub_win, COLOR_PAIR(3));
                // mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                // wattroff(menu_sub_win, COLOR_PAIR(3));
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, 0, " %2d: ", i + 1);
                wattroff(menu_sub_win, COLOR_PAIR(3));
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(6));
                getyx(menu_sub_win, y, x);
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
                wattroff(menu_sub_win, COLOR_PAIR(3));
            } else {
                // mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                mvwprintw(menu_sub_win, i, 0, " %2d: ", i + 1);
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(5));
                getyx(menu_sub_win, y, x);
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
            }
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_F(5):
            werase(*pr_win);
            clear();
            refresh();
            break;
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
#if 0
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
#endif
        }
    }
}

static int menu_args_input_print(char *buf, int size, const char *menu_des)
{
    int i = 0;
    int x, y;
    int h, w;
    int pos = 0;
    //unsigned char number[8];
    unsigned int num;
    WINDOW *input_title;
    WINDOW *input_border;
    WINDOW *input_box;
    WINDOW *input_menu;
    WINDOW *bottom_3d;
    WINDOW *right_3d;

    if (!buf)
        return -1;

    input_menu = newwin(15, MAX_MENU_TITLE_W, 0, 0);
    input_title = subwin(input_menu, 5, 30, 3, 10);
    input_border = subwin(input_menu, 3, 30, 8, 10);
    input_box = subwin(input_border, 1, 28, 9, 11);
    keypad(input_box, TRUE);
    bottom_3d = subwin(input_menu, 1, 30, 11, 11);
    right_3d = subwin(input_menu, 8, 1, 4, 40);

    wbkgd(input_menu, COLOR_PAIR(3));
    werase(input_title);
    box(input_title, 0, 0);
    wbkgd(input_title, COLOR_PAIR(2));
    werase(input_border);
    box(input_border, 0, 0);
    wbkgd(input_border, COLOR_PAIR(2));
    wbkgd(input_box, COLOR_PAIR(2));
    werase(bottom_3d);
    wbkgd(bottom_3d, COLOR_BLACK);
    werase(right_3d);
    wbkgd(right_3d, COLOR_BLACK);

    h = getmaxy(input_title);
    w = getmaxx(input_title);
    wattron(input_title, COLOR_PAIR(4));
    mvwvline(input_title, 0, 0, ACS_VLINE, h);
    mvwhline(input_title, 0, 0, ACS_HLINE, w - 1);
    mvwaddch(input_title, 0, 0, ACS_ULCORNER);
    mvwaddch(input_title, h - 1, 0, ACS_LLCORNER);
    wattroff(input_title, COLOR_PAIR(4));
    h = getmaxy(input_border);
    w = getmaxx(input_border);
    wattron(input_border, COLOR_PAIR(4));
    mvwvline(input_border, 0, w - 1, ACS_VLINE, h);
    mvwhline(input_border, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(input_border, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(input_border, 0, w - 1, ACS_URCORNER);
    wattroff(input_border, COLOR_PAIR(4));

    while (1) {
        h = getmaxy(input_title);
        w = getmaxx(input_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(input_title, COLOR_PAIR(5));
            mvwprintw(input_title, y, x, "%s", menu_des);
            wattroff(input_title, COLOR_PAIR(5));
        }

        touchwin(input_menu);
        wrefresh(input_menu);

        touchwin(input_title);
        wrefresh(input_title);

        touchwin(input_border);
        wrefresh(input_border);

        num = wgetch(input_box);
        switch (num) {
        case KEY_F(5):      // F5 key
            memset(buf, 0x00, sizeof(size));
            werase(input_box);
            i = 0;
            wrefresh(input_box);
            clear();
            refresh();
            break;
        case 10:    // enter
            buf[i + 1] = '\0';
            return 0;
        case 27:    // esc
            clear();
            refresh();
            return -1;
        default:
            if (i >= (size - 1)) {
                memset(buf, 0x00, size);
                werase(input_box);
                i = 0;
                //number[i] = num;
            }
            buf[i++] = num;
            getyx(input_box, y, x);
            mvwprintw(input_box, 0, x, "%c", num);
            wrefresh(input_box);
            break;
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

int menu_args_input_exec(char *buf, int size, const char *menu_des)
{
    clear();
    refresh();
    return menu_args_input_print(buf, size, menu_des);
}

void menu_init(void)
{
    system("dmesg -n 1");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_RED, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, 15, COLOR_WHITE);
    init_pair(5, COLOR_BLUE, COLOR_WHITE);
    init_pair(6, COLOR_YELLOW, COLOR_BLUE);

    // mousemask(ALL_MOUSE_EVENTS, NULL);
    // mouseinterval(0);
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
    wbkgd(menu_title, COLOR_PAIR(2));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(2));
    wbkgd(menu_sub_win, COLOR_PAIR(2));

    h = getmaxy(menu_title);
    w = getmaxx(menu_title);
    wattron(menu_title, COLOR_PAIR(4));
    mvwvline(menu_title, 0, 0, ACS_VLINE, h);
    mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
    mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
    mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
    wattroff(menu_title, COLOR_PAIR(4));
    h = getmaxy(menu_win);
    w = getmaxx(menu_win);
    wattron(menu_win, COLOR_PAIR(4));
    mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
    mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
    mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
    mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
    wattroff(menu_win, COLOR_PAIR(4));

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
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
                wattroff(menu_sub_win, COLOR_PAIR(3));
            } else
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_F(5):
            clear();
            refresh();
            break;
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
    char tmp[2048];
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