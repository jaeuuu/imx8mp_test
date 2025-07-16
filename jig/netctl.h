#ifndef NETWORK_H_
#define NETWORK_H_

struct network_info {
    char ip[32];
    char gw[32];
    char sub[32];
};

int net_ctrl(void);

#endif
