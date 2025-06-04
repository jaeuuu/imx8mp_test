#ifndef LIST_H__
#define LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct st_list {
    void *data;
    list_t *netx;
} list_t;

void *add_list(list_t *head, void *data);

#endif
