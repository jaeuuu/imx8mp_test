#ifndef STATUS_H__
#define STATUS_H__

#define MAX_STS_STRING_SIZE     64

typedef struct st_status {
    int value;
    char str_value[MAX_STS_STRING_SIZE];
} sts_t;

typedef enum STATUS_ARRAY_INDEX {
    STS_CONN_LINE = 0,
    STS_CONN_VPN_SERV,
    STS_CONN_VPN_CLNT,
    STS_LTE_ANT,
    STS_DMB_ANT,
    STS_FM_ANT,
    STS_LTE_RSSI,
    STS_LTE_BER,
    STS_DMB_RSSI,
    STS_DMB_BER,
    STS_FM_RSSI,
    STS_FM_BER,
    STS_VERSION_UI,
    STS_VERSION_MAIN,
    MAX_STS_SIZE = STS_VERSION_MAIN,
} sts_idx_t;

void init_status(void);
int set_status(sts_idx_t idx, int value);
int get_status(sts_idx_t idx);
int set_status_str(sts_idx_t idx, char *str_value, int str_size);
int get_status_str(sts_idx_t idx, char *str_buf, int str_size);

#endif
