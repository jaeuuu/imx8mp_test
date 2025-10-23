#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <poll.h>

int main(void)
{
    const char *dev = "/dev/input/event1";
    struct input_event e;
    int fd, ret;
    struct pollfd pfd;

    fd = open(dev, O_RDONLY);
    if (fd < 0)
        return -1;

    pfd.fd = fd;
    pfd.events = POLLIN;

    while (1) {
        ret = poll(&pfd, 1, -1);

        if (ret < 0)
            return -1;

        if (ret == 0)
            continue;

        if (pfd.revents & POLLIN) {
            ret = read(pfd.fd, &e, sizeof(e));

            if (ret < 0)
                return -1;

            if (e.type == EV_REL && e.code == REL_X) {
                if (e.value > 0)
                    printf("CW(+)\n");
                else if (e.value < 0)
                    printf("CCW(-)\n");
            }

            if (e.type == EV_KEY && e.code == BTN_0) {
                if (e.value > 0)
                    printf("BTN Pressed\n");
                else
                    printf("BTN Released\n");
            }
        }
    }
}