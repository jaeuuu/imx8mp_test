#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/can.h>
#include <linux/can/isotp.h>
#include <linux/can/raw.h>
#include <stdbool.h>
#include <poll.h>
#include <api/menu.h>
#include <errno.h>
#include <semaphore.h>
#include "canctl.h"

//static int sock_stm32_1 = -1;
//static int sock_stm32_2 = -1;
#define MAX_CAN_MENU_DEPTH     3
static WINDOW *pr_win_can[MAX_CAN_MENU_DEPTH];
static int pr_win_can_depth = 0;

//int init_can(int *cansock, const char *ifname, int tx_id, int rx_id, __u8 bs, __u8 stmin)
int init_can(int *cansock, const char *ifname, int tx_id, int rx_id)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_isotp_options opts;
    struct can_isotp_fc_options fcopts;
    //struct can_isotp_ll_options opts;
    __u8 bs = 1;
    __u8 stmin = 0x64;

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

    //fcopts.bs = 8;
    fcopts.bs = bs;
    fcopts.stmin = stmin;        // 9600bps, 100ms
    fcopts.wftmax = 0;

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts)) < 0) {
        perror("setsockopt()");
        close(*cansock);
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
        close(*cansock);
        return -1;
    }

    return 0;
}

//int init_can_fd(int *cansock, const char *ifname, int tx_id, int rx_id, __u8 bs, __u8 stmin)
int init_can_fd(int *cansock, const char *ifname, int tx_id, int rx_id)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_isotp_options opts;
    struct can_isotp_fc_options fcopts;
    struct can_isotp_ll_options llopts;
    __u8 bs = 1;
    __u8 stmin = 0x64;

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

    //fcopts.bs = 8;
    fcopts.bs = bs;
    fcopts.stmin = stmin;
    fcopts.wftmax = 0;

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts)) < 0) {
        perror("setsockopt()");
        close(*cansock);
        return -1;
    }

    memset(&llopts, 0, sizeof(llopts));
    llopts.mtu = CANFD_MTU;       // MTU size
    llopts.tx_dl = 64;            // max dlc size
    llopts.tx_flags = CANFD_BRS;  // Block size
    opts.flags = CAN_ISOTP_TX_PADDING | CAN_ISOTP_RX_PADDING;
    opts.rxpad_content = 0xfc;
    opts.txpad_content = 0xcf;

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_LL_OPTS, &llopts, sizeof(llopts)) < 0) {
        perror("setsockopt(): ");
        close(*cansock);
        return -1;
    }

    if (setsockopt(*cansock, SOL_CAN_ISOTP, CAN_ISOTP_OPTS, &opts, sizeof(opts)) < 0) {
        perror("setsockopt(): ");
        close(*cansock);
        return -1;
    }

    if (bind(*cansock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind(): ");
        close(*cansock);
        return -1;
    }

    return 0;
}

sem_t can_sem;
int send_can(int fd, char *data, int size)
{
    int ret;
    int retry = 0;

    if (fd < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "fd < 0, %s\n", strerror(errno));
        return -1;
    }

    //sem_wait(&can_sem);
    while (++retry <= 1) {
        ret = write(fd, data, size);
        if (ret < 0) {
            pr_win(pr_win_can[pr_win_can_depth], "write(): %s\n", strerror(errno));
            //sem_post(&can_sem);
            return -1;
        }
        //usleep(1 * 1000);
    }
    //sem_post(&can_sem);

    return ret;
}

int recv_can(int fd, char *data, int size)
{
    int ret;

    if (fd < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "fd < 0, %s\n", strerror(errno));
        return -1;
    }

    ret = read(fd, data, size);
    if (ret < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "read() %s\n", strerror(errno));
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

#if 0
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
#endif
#define for_each_cantp_id_map(s, e) \
    for (int i = s; i < e; i++)
static bool is_fd[2] = { false, false };
static bool is_enabled[2] = { false, false };

struct st_can_tx_data {
    int fd;
    __u8 intf;
    __u8 buf[4095];
    __u32 size;
};

static void can_tx_data_thread(struct st_can_tx_data *tx_data)
{
    if (send_can(tx_data->fd, tx_data->buf, tx_data->size) < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "[can%d]send_can() %s\n", tx_data->intf, strerror(errno));
    }
    free(tx_data);
}

static int can_trx_thread(void *intf)
{
    int fd[64];
    struct pollfd pfd[64];
    bool *_is_fd;
    int ret;
    int intf_num;

    if (!intf) {
        pr_win(pr_win_can[pr_win_can_depth], "[%s]interface is NULL!\n", intf);
        return 0;
    }
    //pr_win(pr_win_can[pr_win_can_depth], "[%s]can trx thread on!\n", intf);

    if (!strcmp(intf, "can0")) {
        _is_fd = &is_fd[0];
        intf_num = 0;
    } else if (!strcmp(intf, "can1")) {
        _is_fd = &is_fd[1];
        intf_num = 1;
    } else {
        pr_win(pr_win_can[pr_win_can_depth], "unknown can interface [%s]\n", intf);
        return 0;
    }

    memset(pfd, 0x00, sizeof(pfd));
    for_each_cantp_id_map(0, 64) {
        if (*_is_fd)
            ret = init_can_fd(&fd[i], intf, i, cantp_id_map(i));
        else
            ret = init_can(&fd[i], intf, i, cantp_id_map(i));

        if (ret < 0)
            return 0;

        pfd[i].fd = fd[i];
        pfd[i].events = POLLIN | POLLERR | POLLHUP;
        pfd[i].revents = 0;

        //pr_win(pr_win_can[pr_win_can_depth], "[%s]cantp id(%d) socket init ok!\n", intf, i);
        //usleep(100 * 1000);
    }

    pr_win(pr_win_can[pr_win_can_depth], "[%s]can trx thread on!\n", intf);

    char buf[4095];
    while (1) {
        ret = poll(pfd, 64, 1000);      // 1s delay

        if (ret < 0) {
            perror("poll()");
            continue;
        }

        if (ret == 0)
            continue;

        for_each_cantp_id_map(0, 64) {
            if (pfd[i].revents & POLLIN) {
                memset(buf, 0x00, sizeof(buf));
                ret = recv_can(pfd[i].fd, buf, sizeof(buf));
                if (ret < 0) {
                    pr_win(pr_win_can[pr_win_can_depth], "[%s]recv_can() %s\n", intf, strerror(errno));
                } else {
                    pr_win(pr_win_can[pr_win_can_depth], "[%s][RECV_CAN][TX_ID: %d][RX_ID: %d][LEN: %d]\n", intf, i, cantp_id_map(i), ret);
                    if (i == 9) {
                        wattron(pr_win_can[pr_win_can_depth], COLOR_PAIR(1));
                        pr_win(pr_win_can[pr_win_can_depth], "[%s][CAN-TO-UART] Receive buffer is full!\n", intf, i, cantp_id_map(i), ret);
                        wattroff(pr_win_can[pr_win_can_depth], COLOR_PAIR(1));
                        pthread_exit(1);
                    }
                    struct st_can_tx_data *tx_data = calloc(1, sizeof(struct st_can_tx_data));
                    tx_data->fd = pfd[i].fd;
                    memcpy(tx_data->buf, buf, ret);
                    tx_data->size = ret;
                    tx_data->intf = intf_num;
                    // ret = send_can(pfd[i].fd, buf, ret);
                    // if (ret < 0) {
                    //     pr_win(pr_win_can[pr_win_can_depth], "[%s]send_can() %s\n", intf, strerror(errno));
                    // }
                    pthread_t tx_thread;
                    pthread_create(&tx_thread, NULL, can_tx_data_thread, tx_data);
                }
                //pr_win(pr_win_can[pr_win_can_depth], "[RECV_CAN][ID: %d][LEN: %d]\n", cantp_id_map(i), ret);
#if 0
                for (int j = 0; j < ret; j++) {
                    printf("%02X ", rcv[j]);
                }
#endif
            }

            if (pfd[i].revents & POLLERR) {
                pr_win(pr_win_can[pr_win_can_depth], "[%s]POLLERR, index[%d], %s\n", intf, i, strerror(errno));
                close(pfd[i].fd);
                usleep(100 * 1000);
                if (*_is_fd)
                    ret = init_can_fd(&fd[i], intf, i, cantp_id_map(i));
                else
                    ret = init_can(&fd[i], intf, i, cantp_id_map(i));

                if (ret < 0)
                    return 0;

                pfd[i].fd = fd[i];
                pfd[i].events = POLLIN | POLLERR | POLLHUP;
                pfd[i].revents = 0;
            }

            if (pfd[i].revents & POLLHUP) {
                pr_win(pr_win_can[pr_win_can_depth], "[%s]POLLHUP, index[%d], %s\n", intf, i, strerror(errno));
                close(pfd[i].fd);
                usleep(100 * 1000);
                if (*_is_fd)
                    ret = init_can_fd(&fd[i], intf, i, cantp_id_map(i));
                else
                    ret = init_can(&fd[i], intf, i, cantp_id_map(i));

                if (ret < 0)
                    return 0;

                pfd[i].fd = fd[i];
                pfd[i].events = POLLIN | POLLERR | POLLHUP;
                pfd[i].revents = 0;
            }
        }
    }
    return 0;
}

static int can_up(const char *cmd)
{
    if (!cmd) {
        pr_win(pr_win_can[pr_win_can_depth], "Command is NULL!\n");
        return 0;
    }
    system(cmd);

    char tmp[256];
    FILE *fp = popen("ip link show up | grep can", "r");

    if (fp < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "popen() error!\n");
        return 0;
    }

    memset(tmp, 0x00, sizeof(tmp));
    fread(tmp, sizeof(tmp), 1, fp);
    pr_win(pr_win_can[pr_win_can_depth], "[CAN ENABLED]\n%s\n", tmp);
    pclose(fp);

    return 0;
}

static int can_down(const char *cmd)
{
    if (!cmd) {
        pr_win(pr_win_can[pr_win_can_depth], "Command is NULL!\n");
        return 0;
    }
    system(cmd);

    char tmp[256];
    FILE *fp = popen("ip link show up | grep can", "r");

    if (fp < 0) {
        pr_win(pr_win_can[pr_win_can_depth], "popen() error!\n");
        return 0;
    }

    memset(tmp, 0x00, sizeof(tmp));
    fread(tmp, sizeof(tmp), 1, fp);
    pr_win(pr_win_can[pr_win_can_depth], "[CAN DISABLED]\n%s\n", tmp);
    pclose(fp);

    return 0;
}

static int can_fd_enable(const char *intf)
{
    if (!strcmp(intf, "can0")) {
        can_down("ip link set can0 down");
        can_up("ip link set can0 up type can bitrate 1000000 dbitrate 2000000 fd on loopback off");
        is_fd[0] = true;
        is_enabled[0] = true;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can0 up type can bitrate 1000000 dbitrate 2000000 fd on"},
            {can_down, "DISABLE", "ip link set can0 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN FD ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    } else if (!strcmp(intf, "can1")) {
        can_down("ip link set can1 down");
        can_up("ip link set can1 up type can bitrate 1000000 dbitrate 2000000 fd on loopback off");
        is_fd[1] = true;
        is_enabled[1] = true;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can1 up type can bitrate 1000000 dbitrate 2000000 fd on"},
            {can_down, "DISABLE", "ip link set can1 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN FD ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    }

    return 0;
}

static int can_classic_enable(const char *intf)
{
    if (!strcmp(intf, "can0")) {
        can_down("ip link set can0 down");
        can_up("ip link set can0 up type can bitrate 1000000 loopback off");
        is_fd[0] = false;
        is_enabled[0] = true;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can0 up type can bitrate 1000000"},
            {can_down, "DISABLE", "ip link set can0 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN 2.0 ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    } else if (!strcmp(intf, "can1")) {
        can_down("ip link set can1 down");
        can_up("ip link set can1 up type can bitrate 1000000 loopback off");
        is_fd[1] = false;
        is_enabled[1] = true;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can1 up type can bitrate 1000000"},
            {can_down, "DISABLE", "ip link set can1 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN 2.0 ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    }

    return 0;
}

static int can_disable(const char *intf)
{
    if (!strcmp(intf, "can0")) {
        can_down("ip link set can0 down");
        is_fd[0] = false;
        is_enabled[0] = false;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can0 up type can bitrate 1000000"},
            {can_down, "DISABLE", "ip link set can0 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN 2.0 ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    } else if (!strcmp(intf, "can1")) {
        can_down("ip link set can1 down");
        is_fd[1] = false;
        is_enabled[1] = false;
        /*
        menu_args_t can_clas_en_menu[] = {
            {can_up, "ENABLE", "ip link set can1 up type can bitrate 1000000"},
            {can_down, "DISABLE", "ip link set can1 down"},
            {back2, "back", ""}
        };
        pr_win_can_depth++;
        menu_args_exec(can_clas_en_menu, sizeof(can_clas_en_menu) / sizeof(menu_args_t), "CAN 2.0 ENABLE MENU", &pr_win_can[pr_win_can_depth]);
        pr_win_can_depth--;
        */
    }

    return 0;
}

static int can_enable_loopback(const char *intf)
{
    if (!strcmp(intf, "can0")) {
        can_down("ip link set can0 down");
        if (is_fd[0]) {
            can_up("ip link set can0 up type can bitrate 1000000 dbitrate 2000000 fd on loopback on");
            is_fd[0] = true;
        } else {
            can_up("ip link set can0 up type can bitrate 1000000 loopback on");
            is_fd[0] = false;
        }
    } else if (!strcmp(intf, "can1")) {
        can_down("ip link set can1 down");
        if (is_fd[1]) {
            can_up("ip link set can1 up type can bitrate 1000000 dbitrate 2000000 fd on loopback on");
            is_fd[1] = true;
        } else {
            can_up("ip link set can1 up type can bitrate 1000000 loopback on");
            is_fd[1] = false;
        }
    }
    return 0;
}


static pthread_t can_trx_pthread[2];
static int can_trx_test_on(const char *intf)
{
    char *_intf = calloc(1, strlen(intf) + 1);

    if (!_intf) {
        pr_win(pr_win_can[pr_win_can_depth], "calloc() fail\n");
        return 0;
    }
    strcpy(_intf, intf);

    if (!strcmp(_intf, "can0")) {
        if (pthread_create(&can_trx_pthread[0], NULL, can_trx_thread, _intf) < 0) {
            pr_win(pr_win_can[pr_win_can_depth], "pthread_create() fail\n");
            free(_intf);
        } else
            pthread_detach(can_trx_pthread[0]);
    } else if (!strcmp(_intf, "can1")) {
        if (pthread_create(&can_trx_pthread[1], NULL, can_trx_thread, _intf) < 0) {
            pr_win(pr_win_can[pr_win_can_depth], "pthread_create() fail\n");
            free(_intf);
        } else
            pthread_detach(can_trx_pthread[1]);
    } else {
        pr_win(pr_win_can[pr_win_can_depth], "unknown can interface [%s]\n", _intf);
        free(_intf);
    }

    return 0;
}

static int can_trx_test_off(const char *intf)
{
    if (!strcmp(intf, "can0") && can_trx_pthread[0]) {
        pthread_cancel(can_trx_pthread[0]);
        can_trx_pthread[0] = 0;
    } else if (!strcmp(intf, "can1") && can_trx_pthread[1]) {
        pthread_cancel(can_trx_pthread[1]);
        can_trx_pthread[1] = 0;
    } else {
        pr_win(pr_win_can[pr_win_can_depth], "unknown can interface [%s] or pthread_id is \'0\'\n", intf);
    }

    return 0;
}

static int can_trx_test(void *intf)
{
    menu_args_t can_trx_test_menu[] = {
        {can_trx_test_on, "ON", intf},
        {can_trx_test_off, "OFF", intf},
        {back2, "back", ""}
    };

    pr_win_can_depth++;
    menu_args_exec(can_trx_test_menu, sizeof(can_trx_test_menu) / sizeof(menu_args_t), "CAN TRX TEST MENU", &pr_win_can[pr_win_can_depth]);
    pr_win_can_depth--;

    return 0;
}


static int can_enable(void *intf)
{
    char cmd[32];

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd, "ip link set %s down", (char *)intf);
    menu_args_t can_enable_menu[] = {
        {can_classic_enable, "CAN 2.0 ENABLE", intf},
        {can_fd_enable, "CAN FD ENABLE", intf},
        {can_disable, "CAN DISABLE", intf},
        {back2, "back", ""}
    };

    pr_win_can_depth++;
    menu_args_exec(can_enable_menu, sizeof(can_enable_menu) / sizeof(menu_args_t), "CAN ENABLE MENU", &pr_win_can[pr_win_can_depth]);
    pr_win_can_depth--;
}

static int can_dump(void *intf)
{
    char cmd[32];

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd, "candump %s", (char *)intf);
    //pr_win(pr_win_can[pr_win_can_depth], "Running command: %s\n", cmd);
    endwin();
    system("clear");
    system(cmd);

    clear();
    refresh();
    return 0;
}

static int can_send(void *intf)
{
    char tmp[16];
    char des[64];
    char cmd[1024];
    char *data_8 = "0123456789abcdef";      // 8 characters
    char *data_64 = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"; // 64 characters
    __u32 id;

    sprintf(des, "Input %s ID (0 ~ 1023)", (char *)intf);
    memset(tmp, 0x00, sizeof(tmp));
    if (menu_args_input_exec(tmp, sizeof(tmp), des) < 0)
        return 0;

    id = atoi(tmp);
    if (id < 0 || id > 1024) {
        pr_win(pr_win_can[pr_win_can_depth], "Invalid ID: [%d], should be (0 ~ 1023)\n", id);
        return 0;
    }

    memset(cmd, 0x00, sizeof(cmd));
    if (!strcmp((char *)intf, "can0")) {
        if (!is_enabled[0]) {
            pr_win(pr_win_can[pr_win_can_depth], "%s Interface is disabled.\n", (char *)intf);
            return 0;
        }
        if (is_fd[0])
            sprintf(cmd, "while true; do cansend %s %03X##1%s; sleep 1; done &", (char *)intf, id, data_64);
        else
            sprintf(cmd, "while true; do cansend %s %03X#%s; sleep 1; done &", (char *)intf, id, data_8);
    } else if (!strcmp((char *)intf, "can1")) {
        if (!is_enabled[1]) {
            pr_win(pr_win_can[pr_win_can_depth], "%s Interface is disabled.\n", (char *)intf);
            return 0;
        }
        if (is_fd[1])
            sprintf(cmd, "while true; do cansend %s %03X##1%s; sleep 1; done &", (char *)intf, id, data_64);
        else
            sprintf(cmd, "while true; do cansend %s %03X#%s; sleep 1; done &", (char *)intf, id, data_64);
    } else {
        pr_win(pr_win_can[pr_win_can_depth], "Unknown CAN interface [%s]\n", (char *)intf);
        return 0;
    }

    endwin();
    system("clear");
    system(cmd);

    clear();
    refresh();
    return 0;
}

// static int back_on_can(void *arg)
// {
//     system("killall candump > /dev/null 2>&1");
//     return -1;
// }

static int can_loopback(void *intf)
{
    char cmd[1024];
    char *data_8 = "0123456789abcdef";      // 8 characters
    char *data_64 = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"; // 64 characters

    can_enable_loopback((char *)intf);

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd, "candump %s &", (char *)intf);
    //pr_win(pr_win_can[pr_win_can_depth], "Running command: %s\n", cmd);
    //system("clear");
    system(cmd);

    memset(cmd, 0x00, sizeof(cmd));
    if (!strcmp((char *)intf, "can0")) {
        if (is_fd[0])
            sprintf(cmd, "while true; do cansend %s 123##1%s; sleep 1; done;", (char *)intf, data_64);
        else
            sprintf(cmd, "while true; do cansend %s 123#%s; sleep 1; done;", (char *)intf, data_8);
    } else if (!strcmp((char *)intf, "can1")) {
        if (is_fd[1])
            sprintf(cmd, "while true; do cansend %s 456##1%s; sleep 1; done;", (char *)intf, data_64);
        else
            sprintf(cmd, "while true; do cansend %s 456#%s; sleep 1; done;", (char *)intf, data_64);
    } else {
        pr_win(pr_win_can[pr_win_can_depth], "Unknown CAN interface [%s]\n", (char *)intf);
        return 0;
    }

    endwin();
    system("clear");
    system(cmd);

    system("killall candump");

    clear();
    refresh();
    return 0;
}

static int back_in_can(void *intf)
{
    system("killall sh");
    return -1;
}

static menu_args_t can_ctl_menu[] = {
    {can_enable, "CAN1 ENABLE", "can0"},
    {can_enable, "CAN2 ENABLE", "can1"},
    //{can_trx_test, "CAN1 TRX TEST", "can0"},
    //{can_trx_test, "CAN2 TRX TEST", "can1"},
    {can_dump, "CAN1 RECV", "can0"},
    {can_dump, "CAN2 RECV", "can1"},
    {can_send, "CAN1 SEND", "can0"},
    {can_send, "CAN2 SEND", "can1"},
    {can_loopback, "CAN1 LOOPBACK", "can0"},
    {can_loopback, "CAN2 LOOPBACK", "can1"},
    {back_in_can, "back", ""},
};

void can_init(void)
{
    sem_init(&can_sem, 0, 1);
}

int can_ctl(void)
{
    menu_args_exec(can_ctl_menu, sizeof(can_ctl_menu) / sizeof(menu_args_t), "CAN TEST MENU", &pr_win_can[pr_win_can_depth]);
    return 0;
}