#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <api/menu.h>
#include <stdarg.h>
#include <semaphore.h>
#include <locale.h>

/* Color pair list (COLOR_x_y)
 * x: text color
 * y: backdrop color
 */
#define COLOR_RED_WHITE     1
#define COLOR_BLACK_WHITE   2
#define COLOR_WHITE_BLUE    3
#define COLOR_RWHITE_WHITE  4
#define COLOR_BLUE_WHITE    5
#define COLOR_YELLOW_BLUE   6

 /* In my environment, max width 158, max height 60 */

#define CONSOLE_TOTAL_W     120
#define CONSOLE_TOTAL_H     50

#define MAX_MENU_TITLE_W    50
#define MAX_MENU_TITLE_H    5

#define MAIN_MENU_START_X   0

static sem_t pr_sem;

static bool is_xterm_8 = false;

static void menu_print(menu_t *menus, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    int i;
    int x, y;
    int h, w;
    int pos = 0;
    int ch;
    int cnum = 0;
    char num[3];
    WINDOW *menu_title = NULL;
    WINDOW *menu_sub_title = NULL;
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
    //menu_sub_title = subwin(menu_title, MAX_MENU_TITLE_H -2, MAX_MENU_TITLE_W -2, 1, MAIN_MENU_START_X + 1);
    menu_win = newwin(menu_size + 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H, MAIN_MENU_START_X);     // +1 margin
    menu_sub_win = subwin(menu_win, menu_size, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + 1, MAIN_MENU_START_X + 1);

    /* result box */
    if (pr_win) {
        result_win = newwin(CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2, MAX_MENU_TITLE_W, MAX_MENU_TITLE_H + menu_size + 2, MAIN_MENU_START_X);
        *pr_win = subwin(result_win, CONSOLE_TOTAL_H - MAX_MENU_TITLE_H - menu_size - 2 - 2, MAX_MENU_TITLE_W - 2, MAX_MENU_TITLE_H + menu_size + 2 + 1, MAIN_MENU_START_X + 1);
        scrollok(*pr_win, TRUE);
    }

    box(menu_title, 0, 0);
    wbkgd(menu_title, COLOR_PAIR(COLOR_BLACK_WHITE));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    wbkgd(menu_sub_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    if (pr_win) {
        box(result_win, 0, 0);
        wbkgd(result_win, COLOR_PAIR(COLOR_BLACK_WHITE));
        wbkgd(*pr_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    }
#if 1   /* raised or lowerd option off.. acs charactors issue... */
    if (!is_xterm_8) {
        /* raised block */
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        wattron(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_title, 0, 0, ACS_VLINE, h);
        mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
        //mvwhline(menu_title, 0, 0, '-', w - 1);
        mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
        mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
        wattroff(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        /* lowered block */
        h = getmaxy(menu_win);
        w = getmaxx(menu_win);
        wattron(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
        mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
        //mvwhline(menu_win, h - 1, 1, '-', w - 1);
        mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
        wattroff(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        /* lowered block */
        h = getmaxy(result_win);
        w = getmaxx(result_win);
        wattron(result_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(result_win, 0, w - 1, ACS_VLINE, h);
        mvwhline(result_win, h - 1, 1, ACS_HLINE, w - 1);
        //mvwhline(result_win, h - 1, 1, '-', w - 1);
        mvwaddch(result_win, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(result_win, 0, w - 1, ACS_URCORNER);
        wattroff(result_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
    }
#endif

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(menu_title, COLOR_PAIR(COLOR_BLUE_WHITE));
            mvwprintw(menu_title, y, x, "%s", menu_des);
            wattroff(menu_title, COLOR_PAIR(COLOR_BLUE_WHITE));
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
                /* old */
                // wattron(menu_sub_win, COLOR_PAIR(3));
                // mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                // wattroff(menu_sub_win, COLOR_PAIR(3));
                /* new */
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(COLOR_YELLOW_BLUE));
                getyx(menu_sub_win, y, x);
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
            } else {
                /* old */
                //mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (menus + i)->func_des);
                //getyx(menu_sub_win, y, x);
                /* new */
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(COLOR_BLUE_WHITE));
                getyx(menu_sub_win, y, x);
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
            }
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        ch = getch();
        switch (ch) {
        case KEY_F(5):  // F5 key
            //getmaxyx(stdscr, h, w);
            system("resize");
            resize_term(0, 0);
            //wresize(stdscr, h, w);
            //werase(stdscr);
            cnum = 0;
            if (pr_win)
                werase(*pr_win);
            //resize_term(0, 0);
            clear();
            refresh();
            break;
        case KEY_UP:
            cnum = 0;
            pos = (pos - 1 + menu_size) % menu_size;
            break;
        case KEY_DOWN:
            cnum = 0;
            pos = (pos + 1 + menu_size) % menu_size;
            break;
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
            num[cnum++] = ch;
            if (cnum > 2)
                cnum = 0;
            break;
        case 10:    // enter
            if (cnum > 0) {
                num[cnum] = '\0';
                char tmp_pos = atoi(num) - 1;
                cnum = 0;
                if (tmp_pos + 1 > menu_size)
                    break;
                pos = tmp_pos;
            }

            if ((menus + pos)->func() < 0) {
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
                return;
            }
            break;
        }
    }
}

static void menu_args_print(menu_args_t *menus, int menu_size, const char *menu_des, WINDOW **pr_win)
{
    int i;
    int x, y;
    int h, w;
    int pos = 0;
    int ch;
    int cnum = 0;
    char num[3];
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
    wbkgd(menu_title, COLOR_PAIR(COLOR_BLACK_WHITE));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    wbkgd(menu_sub_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    if (pr_win) {
        box(result_win, 0, 0);
        wbkgd(result_win, COLOR_PAIR(COLOR_BLACK_WHITE));
        wbkgd(*pr_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    }

#if 1   /* acs issue */
    if (!is_xterm_8) {
        /* raised block */
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        wattron(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_title, 0, 0, ACS_VLINE, h);
        mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
        mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
        mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
        wattroff(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        /* lowered block */
        h = getmaxy(menu_win);
        w = getmaxx(menu_win);
        wattron(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
        mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
        mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
        wattroff(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        /* lowered block */
        h = getmaxy(result_win);
        w = getmaxx(result_win);
        wattron(result_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(result_win, 0, w - 1, ACS_VLINE, h);
        mvwhline(result_win, h - 1, 1, ACS_HLINE, w - 1);
        mvwaddch(result_win, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(result_win, 0, w - 1, ACS_URCORNER);
        wattroff(result_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
    }
#endif

    while (1) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(menu_title, COLOR_PAIR(COLOR_BLUE_WHITE));
            mvwprintw(menu_title, y, x, "%s", menu_des);
            wattroff(menu_title, COLOR_PAIR(COLOR_BLUE_WHITE));
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
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(COLOR_YELLOW_BLUE));
                getyx(menu_sub_win, y, x);
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
            } else {
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (menus + i)->func_des[0] | COLOR_PAIR(COLOR_BLUE_WHITE));
                getyx(menu_sub_win, y, x);
                mvwprintw(menu_sub_win, i, x, "%s\n", &((menus + i)->func_des[1]));
            }
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        ch = getch();
        switch (ch) {
        case KEY_F(5):
            system("resize");
            resize_term(0, 0);
            cnum = 0;
            werase(*pr_win);
            //resize_term(0, 0);
            clear();
            refresh();
            break;
        case KEY_UP:
            cnum = 0;
            pos = (pos - 1 + menu_size) % menu_size;
            break;
        case KEY_DOWN:
            cnum = 0;
            pos = (pos + 1 + menu_size) % menu_size;
            break;
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
            num[cnum++] = ch;
            if (cnum > 2)
                cnum = 0;
            break;
        case 10:    // enter
            if (cnum > 0) {
                num[cnum] = '\0';
                char tmp_pos = atoi(num) - 1;
                cnum = 0;
                if (tmp_pos + 1 > menu_size)
                    break;
                pos = tmp_pos;
            }

            if ((menus + pos)->func((menus + pos)->args) < 0) {
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
                return;
            }
            break;
        }
    }
}

static int menu_args_input_print(char *buf, int size, const char *menu_des)
{
    int i = 0;
    int x, y;
    int h, w;
    int pos = 0;
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

    wbkgd(input_menu, COLOR_PAIR(COLOR_WHITE_BLUE));
    werase(input_title);
    box(input_title, 0, 0);
    wbkgd(input_title, COLOR_PAIR(COLOR_BLACK_WHITE));
    werase(input_border);
    box(input_border, 0, 0);
    wbkgd(input_border, COLOR_PAIR(COLOR_BLACK_WHITE));
    wbkgd(input_box, COLOR_PAIR(COLOR_BLACK_WHITE));
    werase(bottom_3d);
    wbkgd(bottom_3d, COLOR_BLACK);
    werase(right_3d);
    wbkgd(right_3d, COLOR_BLACK);

#if 1   /* acs issue */
    if (!is_xterm_8) {
        /* raised block */
        h = getmaxy(input_title);
        w = getmaxx(input_title);
        wattron(input_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(input_title, 0, 0, ACS_VLINE, h);
        mvwhline(input_title, 0, 0, ACS_HLINE, w - 1);
        mvwaddch(input_title, 0, 0, ACS_ULCORNER);
        mvwaddch(input_title, h - 1, 0, ACS_LLCORNER);
        wattroff(input_title, COLOR_PAIR(COLOR_RWHITE_WHITE));

        /* lowered bloack */
        h = getmaxy(input_border);
        w = getmaxx(input_border);
        wattron(input_border, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(input_border, 0, w - 1, ACS_VLINE, h);
        mvwhline(input_border, h - 1, 1, ACS_HLINE, w - 1);
        mvwaddch(input_border, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(input_border, 0, w - 1, ACS_URCORNER);
        wattroff(input_border, COLOR_PAIR(COLOR_RWHITE_WHITE));
    }
#endif

    while (1) {
        h = getmaxy(input_title);
        w = getmaxx(input_title);
        y = h / 2;
        x = w / 2;
        if (menu_des) {
            x -= strlen(menu_des) / 2;
            wattron(input_title, COLOR_PAIR(COLOR_BLUE_WHITE));
            mvwprintw(input_title, y, x, "%s", menu_des);
            wattroff(input_title, COLOR_PAIR(COLOR_BLUE_WHITE));
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
            system("resize");
            resize_term(0, 0);
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

void menu_init(const char *term_type)
{
    if (term_type) {
        if (!strcmp(term_type, "xterm")) {
            setenv("TERM", term_type, 1);
            is_xterm_8 = true;
        } else {
            setenv("TERM", "xterm-256color", 1);
            is_xterm_8 = false;
        }
    } else {
        setenv("TERM", "xterm-256color", 1);
        is_xterm_8 = false;
    }
    system("dmesg -n 1");
    //setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    start_color();
    init_pair(COLOR_RED_WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(COLOR_BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_RWHITE_WHITE, 15, COLOR_WHITE);
    init_pair(COLOR_BLUE_WHITE, COLOR_BLUE, COLOR_WHITE);
    init_pair(COLOR_YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);

    sem_init(&pr_sem, 0, 1);

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
    {is_yes, "YES"},
    {is_no, "NO"},
};

int menu_exit(void)
{
    char *des = "ARE YOU SURE YOU WANT TO QUIT?";
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
    wbkgd(menu_title, COLOR_PAIR(COLOR_BLACK_WHITE));
    box(menu_win, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(COLOR_BLACK_WHITE));
    wbkgd(menu_sub_win, COLOR_PAIR(COLOR_BLACK_WHITE));

#if 1   /* acs issue.. */
    if (!is_xterm_8) {
        h = getmaxy(menu_title);
        w = getmaxx(menu_title);
        wattron(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_title, 0, 0, ACS_VLINE, h);
        mvwhline(menu_title, 0, 0, ACS_HLINE, w - 1);
        mvwaddch(menu_title, 0, 0, ACS_ULCORNER);
        mvwaddch(menu_title, h - 1, 0, ACS_LLCORNER);
        wattroff(menu_title, COLOR_PAIR(COLOR_RWHITE_WHITE));
        h = getmaxy(menu_win);
        w = getmaxx(menu_win);
        wattron(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
        mvwvline(menu_win, 0, w - 1, ACS_VLINE, h);
        mvwhline(menu_win, h - 1, 1, ACS_HLINE, w - 1);
        mvwaddch(menu_win, h - 1, w - 1, ACS_LRCORNER);
        mvwaddch(menu_win, 0, w - 1, ACS_URCORNER);
        wattroff(menu_win, COLOR_PAIR(COLOR_RWHITE_WHITE));
    }
#endif

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
                /*
                wattron(menu_sub_win, COLOR_PAIR(3));
                mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
                wattroff(menu_sub_win, COLOR_PAIR(3));
                */
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (exit_menu + i)->func_des[0] | COLOR_PAIR(COLOR_YELLOW_BLUE));
                getyx(menu_sub_win, y, x);
                wattron(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
                mvwprintw(menu_sub_win, i, x, "%s\n", &((exit_menu + i)->func_des[1]));
                wattroff(menu_sub_win, COLOR_PAIR(COLOR_WHITE_BLUE));
            } else {
                mvwprintw(menu_sub_win, i, 0, " %2d)  ", i + 1);
                getyx(menu_sub_win, y, x);
                mvwaddch(menu_sub_win, i, x, (exit_menu + i)->func_des[0] | COLOR_PAIR(COLOR_BLUE_WHITE));
                getyx(menu_sub_win, y, x);
                mvwprintw(menu_sub_win, i, x, "%s\n", &((exit_menu + i)->func_des[1]));
                //mvwprintw(menu_sub_win, i, 0, " %2d: %s\n", i + 1, (exit_menu + i)->func_des);
            }
        }

        touchwin(menu_sub_win);
        wrefresh(menu_sub_win);

        switch (getch()) {
        case KEY_F(5):
            resize_term(0, 0);
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

    sem_wait(&pr_sem);
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    getyx(win, y, x);
    mvwprintw(win, y, x, "%s", tmp);
    wrefresh(win);
    sem_post(&pr_sem);
}