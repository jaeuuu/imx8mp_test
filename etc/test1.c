#include <ncurses.h>

int main() {
    char str[80];

    // ncurses 초기화
    initscr();              // 화면 초기화
    cbreak();               // 줄 버퍼링 없이 입력
    noecho();               // 입력 시 화면에 표시 안 함
    keypad(stdscr, TRUE);   // 방향키 등 사용 가능

    // 메시지 출력
    while (1) {
    mvprintw(10, 10, "text input:");
    refresh();

    // 사용자 입력 받기
    getstr(str);

    // 입력된 문자열 출력
    mvprintw(12, 10, "input string: %s", str);
    refresh();

    // 사용자 키 입력 대기
    getch();

    // 종료
    endwin();
    }
    return 0;
}

