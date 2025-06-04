#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include "api/debug.h"
#include "utils.h"
#include "status.h"

static sts_t status[MAX_STS_SIZE];
static sem_t sts_sem;

static int sem_init_sts(void)
{
    return sem_init(&sts_sem, 0, 1);
}

static void sem_wait_sts(void)
{
    sem_wait(&sts_sem);
}

static void sem_post_sts(void)
{
    sem_post(&sts_sem);
}

static void sem_destory_sts(void)
{
    sem_destroy(&sts_sem);
}

static int check_status_idx_val(sts_idx_t idx, int value)
{
    int ret = -1;
    int min, max;

    switch (idx) {
    case STS_CONN_LINE:
    case STS_CONN_VPN_SERV:
    case STS_CONN_VPN_CLNT:
        min = 0;
        max = 1;
        ret = check_val_range(value, min, max);
        break;
    case STS_LTE_ANT:
    case STS_DMB_ANT:
    case STS_FM_ANT:
        min = 0;            // antenna low db
        max = 6;            // antenna high db
        ret = check_val_range(value, min, max);
        break;
    case STS_LTE_RSSI:
    case STS_LTE_BER:
    case STS_DMB_RSSI:
    case STS_DMB_BER:
    case STS_FM_RSSI:
    case STS_FM_BER:
    case STS_VERSION_UI:
    case STS_VERSION_MAIN:
        ret = check_val_pass(value);
        break;
    default:
        dlp("error: unknown status index! idx-[%d]", idx);
        break;
    }

    return ret;
}

static int check_status_idx(sts_idx_t idx)
{
    switch (idx) {
    case STS_CONN_LINE:
    case STS_CONN_VPN_SERV:
    case STS_CONN_VPN_CLNT:
    case STS_LTE_ANT:
    case STS_DMB_ANT:
    case STS_FM_ANT:
    case STS_LTE_RSSI:
    case STS_LTE_BER:
    case STS_DMB_RSSI:
    case STS_DMB_BER:
    case STS_FM_RSSI:
    case STS_FM_BER:
    case STS_VERSION_UI:
    case STS_VERSION_MAIN:
        return 0;
    default:
        dlp("error: unknown status index! idx-[%d]", idx);
        return -1;
    }
}

void init_status(void)
{
    memset(status, 0x00, sizeof(status));

    if (sem_init_sts() < 0) {
        dlp("error: sem_init_sts() fail");
        exit(1);
    }
    dlp("note: initialized status!");
}

int set_status(sts_idx_t idx, int value)
{
    if (check_status_idx_val(idx, value) < 0) {
        dlp("error: not valid value! idx-[%d] value-[%d]", idx, value);
        return -1;
    }

    sem_wait_sts();
    status[idx].value = value;
    sem_post_sts();

    return 0;
}

int get_status(sts_idx_t idx)
{
    int ret = -1;

    if (check_status_idx(idx) < 0)
        return ret;
   
    sem_wait_sts();
    ret = status[idx].value;
    sem_post_sts();

    return ret;
}

int set_status_str(sts_idx_t idx, char *str_value, int str_size)
{
    if (check_status_idx(idx) < 0)
        return -1;

    if (!str_value)
        return -1;

    if (str_size < 0 || str_size >= MAX_STS_STRING_SIZE) {
        dlp("error: not valid string size! size-[%d]", str_size);
        return -1;
    }

    sem_wait_sts();
    memset(status[idx].str_value, 0x00, MAX_STS_STRING_SIZE);
    memcpy(status[idx].str_value, str_value, str_size);
    sem_post_sts();

    return 0;
}

int get_status_str(sts_idx_t idx, char *str_buf, int str_buf_size)
{
    if (check_status_idx(idx) < 0)
        return -1;

    if (!str_buf)
        return -1;

    if (str_buf_size < MAX_STS_STRING_SIZE) {
        dlp("error: string size is too small! should be size more than [%d]! size-[%d]", MAX_STS_STRING_SIZE, str_buf_size);
        return -1;
    }
   
    sem_wait_sts();
    memset(str_buf, 0x00, str_buf_size);
    memcpy(str_buf, status[idx].str_value, strlen(status[idx].str_value));
    sem_post_sts();

    return 0;
}

