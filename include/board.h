#ifndef MX8MP_OP_BOARD_H__
#define MX8MP_OP_BOARD_H__

#include <stdio.h>

void __attribute__((weak)) board_init(void)
{
    printf("board_init()\r\n");
}

void __attribute__((weak)) start_board(void)
{
    printf("start_board()\r\n");
}

#endif