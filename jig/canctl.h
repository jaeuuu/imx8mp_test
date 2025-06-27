#ifndef CANCTL_H__
#define CANCTL_H__


int init_can(int *cansock, const char *ifname, int tx_id, int rx_id);
int init_can_fd(int *cansock, const char *ifname);
int send_can(int *cansock, char *data, int size);
int recv_can(int *cansock, char *data, int size);
void close_can(int *cansock);


#endif