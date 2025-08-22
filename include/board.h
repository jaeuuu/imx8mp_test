#ifndef MX8MP_OP_BOARD_H__
#define MX8MP_OP_BOARD_H__

#include <stdio.h>

void __attribute__((weak)) board_init(int argc, char **argv)
{
    printf("board_init()\r\n");
}

void __attribute__((weak)) start_board(int argc, char **argv)
{
    printf("start_board()\r\n");
}

#endif