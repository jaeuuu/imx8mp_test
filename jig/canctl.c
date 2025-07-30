#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/isotp.h>
#include <linux/can/raw.h>
#include <stdbool.h>
#include <poll.h>
#include <api/menu.h>

static int socks[8];
static int sock;
//static int sock_stm32_1 = -1;
//static int sock_stm32_2 = -1;

int init_can(int *cansock, const char *ifname, int tx_id, int rx_id)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_isotp_options opts;
    struct can_isotp_fc_options fcopts;
    //struct can_isotp_ll_options opts;

    *cansock = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    if (*cansock < 0) {
        perror("socket(): ");
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);
    ioctl(*cansock, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = rx_id;     // 상대 ID
    addr.can_addr.tp.tx_id = tx_id;     // 내 ID

    /* 필요시 opt 사용 */
    /*
    memset(&opts, 0, sizeof(opts));
    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_OPTS, &opts, sizeof(opts)) < 0) {
        perror("setsockopt(): ");
        return -1;
    }
    */

    memset(&fcopts, 0x00, sizeof(fcopts));
    //fcopts.bs = 8;
    //fcopts.stmin = 0x01;
    //fcopts.wftmax = 8;

    fcopts.bs = 8;
    fcopts.stmin = 0x01;
    fcopts.wftmax = 0;

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts)) < 0) {
        perror("setsockopt()");
        return -1;
    }
    /*
        int on = 1;
        if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_WAIT_TX_DONE, &on, sizeof(on)) < 0) {
            perror("setsockopt()");
            return -1;
        }
    */

    if (bind(*cansock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind(): ");
        return -1;
    }

    return 0;
}

int init_can_fd(int *cansock, const char *ifname, int tx_id, int rx_id)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    //struct can_isotp_options opts;
    struct can_isotp_fc_options fcopts;
    struct can_isotp_ll_options opts;

    *cansock = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    if (*cansock < 0) {
        perror("socket(): ");
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);
    ioctl(*cansock, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = rx_id;
    addr.can_addr.tp.tx_id = tx_id;

    memset(&fcopts, 0x00, sizeof(fcopts));
    //fcopts.bs = 8;
    //fcopts.stmin = 0x01;
    //fcopts.wftmax = 8;

    fcopts.bs = 8;
    fcopts.stmin = 0x01;
    fcopts.wftmax = 0;

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts)) < 0) {
        perror("setsockopt()");
        return -1;
    }

    memset(&opts, 0, sizeof(opts));
    opts.mtu = CANFD_MTU;       // MTU size
    opts.tx_dl = 64;            // max dlc size
    opts.tx_flags = CANFD_BRS;  // Block size

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_LL_OPTS, &opts, sizeof(opts)) < 0) {
        perror("setsockopt(): ");
        return -1;
    }

    if (bind(*cansock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind(): ");
        return -1;
    }

    return 0;
}

int send_can(int *cansock, char *data, int size)
{
    int ret;

    if (!cansock) {
        printf("cansock is null!\n");
        return -1;
    }

    if (*cansock < 0) {
        printf("cansock < 0\n");
        return -1;
    }

    ret = write(*cansock, data, size);
    if (ret < 0) {
        perror("write(): ");
        return -1;
    }

    return ret;
}

int recv_can(int *cansock, char *data, int size)
{
    int ret;

    if (!cansock) {
        printf("cansock is null!\n");
        return -1;
    }

    if (*cansock < 0) {
        printf("cansock < 0\n");
        return -1;
    }

    ret = read(*cansock, data, size);
    if (ret < 0) {
        perror("read(): ");
        return -1;
    }

    return ret;
}

void close_can(int *cansock)
{
    close(*cansock);
    *cansock = -1;
}

#ifdef CAN_TX_TEST
int main(int argc, char **argv)
{
    bool is_fd;
    int id;
    char payload[8192];
    char rcv[8192];
    int ret;
    int i;

    if (argc != 4) {
        printf("Usage: isotp-test-tx [can0|can1] [1(can_fd)|0(can)] [ID]\n");
        return -1;
    }

    is_fd = atoi(argv[2]);
    id = atoi(argv[3]);
    memset(payload, id, sizeof(payload));

    printf("interface: [%s], can fd: [%s], can id: [%s]\n", argv[1], argv[2], argv[3]);

    if (is_fd)
        init_can_fd(&sock, argv[1], id, id - 8);
    else
        init_can(&sock, argv[1], id, id - 8);

    while (1) {
        ret = send_can(&sock, payload, sizeof(payload));
        if (ret < 0) {
            printf("send_can() fail!\n");
        }
#if 1
        memset(rcv, 0x00, sizeof(rcv));
        ret = recv_can(&sock, rcv, sizeof(rcv));
        if (ret < 0) {
            printf("recv_can() fail!\n");
        }

        printf("[RECV ID(%d) from ID(0)][%d]\n", id, ret);
#if 0
        for (i = 0; i < ret; i++) {
            printf("%02X ", rcv[i]);
        }
#endif
        //printf("\n==================================\n");
#endif
        sleep(1);
    }
}
int main(int argc, char **argv)
{
    bool is_fd;
    char payload[8192];
    char rcv[8192];
    int ret;
    int i;
    struct pollfd ps[8];

    if (argc != 3) {
        printf("Usage: isotp-test-rx [can0|can1] [1(can_fd)|0(can)]\n");
        return -1;
    }

    is_fd = atoi(argv[2]);
    memset(payload, 0x00, sizeof(payload));
    memset(socks, -1, sizeof(socks));
    memset(&ps, 0x00, sizeof(ps));

    printf("interface: [%s], can fd: [%s]\n", argv[1], argv[2]);

    if (is_fd) {
        //init_can_fd(&socks[i], argv[1], i, i + 8);
        for (i = 0; i < 8; i++) {
            init_can_fd(&socks[i], argv[1], i, i + 8);
            ps[i].fd = socks[i];
            ps[i].events = POLLIN | POLLERR | POLLHUP;
            ps[i].revents = 0;
        }
    } else {
        //init_can(&sock, argv[1], 2, 4);
        //init_can(&sock_stm32_1, argv[1], 2, 1);
        //init_can(&sock_stm32_1, argv[1], 2, 3);
        for (i = 0; i < 8; i++) {
            init_can(&socks[i], argv[1], i, i + 8);
            ps[i].fd = socks[i];
            ps[i].events = POLLIN | POLLERR | POLLHUP;
            ps[i].revents = 0;
        }
    }

    while (1) {
#if 0
        ret = send_can(&sock, payload, sizeof(payload));
        if (ret < 0) {
            printf("send_can() fail!\n");
        }
#endif
        ret = poll(ps, 8, 1000);     // 8 fds, 1s

        if (ret < 0) {
            perror("poll()");
            continue;
        }

        if (ret == 0)
            continue;

        for (i = 0; i < 8; i++) {
            if (ps[i].revents & POLLIN) {
                memset(rcv, 0x00, sizeof(rcv));
                ret = recv_can(&socks[i], rcv, sizeof(rcv));
                if (ret < 0) {
                    printf("recv_can() fail!\n");
                    perror("recv_can()");
                } else {
                    ret = send_can(&socks[i], payload, sizeof(payload));
                    if (ret < 0) {
                        printf("send_can() fail\n");
                        perror("send_can()");
                    }
                }

                printf("[RECV ID(0) from ID(%d)][%d]===============\n", i + 1, ret);
#if 0
                for (int j = 0; j < ret; j++) {
                    printf("%02X ", rcv[j]);
                }
#endif
            }

            if (ps[i].revents & POLLERR) {
                //perror("POLLERR [%d]");
                printf("index = %d pollerr\n", i);
            }

            if (ps[i].revents & POLLHUP)
                perror("POLLHUP");
        }
#if 0
        memset(rcv, 0x00, sizeof(rcv));
        ret = recv_can(&sock, rcv, sizeof(rcv));
        if (ret < 0) {
            printf("recv_can() fail!\n");
        } else {
            ret = send_can(&sock, payload, sizeof(payload));
            if (ret < 0) {
                printf("send_can() fail!\n");
            }
        }

        printf("[RECV CAN-ID(2) from ID(4)]==> [len=%d]\n", ret);
        for (i = 0; i < ret; i++) {
            printf("%02X ", rcv[i]);
        }

        memset(rcv, 0x00, sizeof(rcv));
        ret = recv_can(&sock_stm32_1, rcv, sizeof(rcv));
        if (ret < 0) {
            printf("recv_can() fail!\n");
        } else {
            ret = send_can(&sock_stm32_1, payload, sizeof(payload));
            if (ret < 0) {
                printf("send_can() fail!\n");
            }
        }

        printf("[RECV CAN-ID(2) from ID(1)]==> [len=%d]\n", ret);
        for (i = 0; i < ret; i++) {
            printf("%02X ", rcv[i]);
        }

        memset(rcv, 0x00, sizeof(rcv));
        ret = recv_can(&sock_stm32_2, rcv, sizeof(rcv));
        if (ret < 0) {
            printf("recv_can() fail!\n");
        } else {
            ret = send_can(&sock_stm32_2, payload, sizeof(payload));
            if (ret < 0) {
                printf("send_can() fail!\n");
            }
        }

        printf("[RECV CAN-ID(2) from ID(3)]==> [len=%d]\n", ret);
        for (i = 0; i < ret; i++) {
            printf("%02X ", rcv[i]);
        }
        printf("\n==================================\n");
        sleep(3);
#endif
    }
}
#else
#endif

static int can_fd_init(const char *intf)
{
}

static int can_classic_init(const char *intf)
{
}

static menu_t can_ctl_init_menu[] = {
    {can_classic_init, "CAN 2.0"},
    {},
};


static int can_init(void)
{
    menu_args_t can_ctl_init_menu[] = {
        {can_classic_init, "CAN 2.0"}
    }
}

static menu_t can_ctl_menu[] = {
    {can_init, "CAN1 INITIALIZE"},
    {can_init, "CAN2 INITIALIZE"},
    {},
};

int can_ctl(void)
{
}