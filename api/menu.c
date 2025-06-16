#include <stdio.h>
#include <ncurses.h>
#include <api/menu.h>

static void menu_print(menu_t *menus, int menu_size, const char *menu_des)
{
    int i;
    int x, y;
    int pos = 0;

    if (menu_size <= 0) {
        printf("menu_size error! [menu_size: %d]\n", menu_size);
        clear();
        refresh();
        endwin();
        exit(1);
    }

    while (1) {
        if (menu_des)
            mvprintw(0, 0, "\n\n*********************************************** \
                    \n\n%s \
                    \n\n***********************************************\n"
                , menu_des);
        refresh();

        getyx(stdscr, y, x);

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                attron(A_REVERSE);
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (menus + i)->func_des);
                attroff(A_REVERSE);
            } else
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (menus + i)->func_des);
        }

        getyx(stdscr, y, x);
        mvprintw(y, 0, "+---------------------------------------------+\n");
        move(y + 3, 0);

        refresh();

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
                endwin();
                return;
            }
            break;
        case 27:    // esc
            clear();
            refresh();
            endwin();
            exit(0);
        }
    }
}

static void menu_args_print(menu_args_t *menus, int menu_size, const char *menu_des)
{
    int i;
    int x, y;
    int pos = 0;

    if (menu_size <= 0) {
        printf("menu_size error! [menu_size: %d]\n", menu_size);
        clear();
        refresh();
        endwin();
        exit(1);
    }

    while (1) {
        if (menu_des)
            mvprintw(0, 0, "\n\n*********************************************** \
                    \n\n%s \
                    \n\n***********************************************\n"
                , menu_des);
        refresh();

        getyx(stdscr, y, x);

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                attron(A_REVERSE);
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (menus + i)->func_des);
                attroff(A_REVERSE);
            } else
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (menus + i)->func_des);
        }

        getyx(stdscr, y, x);
        mvprintw(y, 0, "+---------------------------------------------+\n");
        move(y + 3, 0);

        refresh();

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
                endwin();
                return;
            }
            break;
        case 27:    // esc
            clear();
            refresh();
            endwin();
            exit(0);
        }
    }
}

void menu_exec(menu_t *menu, int menu_size, const char *menu_des)
{
    clear();
    refresh();
    //char *des = "\t   SAFWI Board Test Program";
    menu_print(menu, menu_size, menu_des);
}

void menu_args_exec(menu_args_t *menu_args, int menu_args_size, const char *menu_des)
{
    clear();
    refresh();
    menu_args_print(menu_args, menu_args_size, menu_des);
}

void menu_init(void)
{
    system("dmesg -n 1");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
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
    char *des = "\tAre you sure you want to quit?";
    int i;
    int x, y;
    int pos = 0;
    int menu_size = sizeof(exit_menu) / sizeof(menu_t);

    while (1) {
        if (des)
            mvprintw(0, 0, "\n\n*********************************************** \
                    \n\n%s \
                    \n\n***********************************************\n"
                , des);
        refresh();

        getyx(stdscr, y, x);

        // menu list
        for (i = 0; i < menu_size; i++) {
            if (i == pos) {
                attron(A_REVERSE);
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (exit_menu + i)->func_des);
                attroff(A_REVERSE);
            } else
                mvprintw(i + y, 0, "| %2d: %s\n", i + 1, (exit_menu + i)->func_des);
        }

        getyx(stdscr, y, x);
        mvprintw(y, 0, "+---------------------------------------------+\n");
        move(y + 3, 0);

        refresh();

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
            endwin();
            if ((exit_menu + pos)->func() < 0)
                return -1;
            return 0;
        case 27:    // esc
            clear();
            refresh();
            endwin();
            exit(0);
        }
    }
}