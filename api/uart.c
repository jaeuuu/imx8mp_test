#include <api/uart.h>
#include <api/debug.h>

static int sem_init_uart(sem_t *sem)
{
    return sem_init(sem, 0, 1);
}

static void sem_wait_uart(sem_t *sem)
{
    sem_wait(sem);
}

static void sem_post_uart(sem_t *sem)
{
    sem_post(sem);
}

static void sem_destroy_uart(sem_t *sem)
{
    sem_destroy(sem);
}

int open_uart(uart_t *uart)
{
    struct termios oldtio, newtio;

    uart->fd = open(uart->dev, O_RDWR | O_NOCTTY);
    if (uart->fd < 0) {
        perror("open() fail");
        return -1;
    }

    tcgetattr(uart->fd, &oldtio);
    newtio = oldtio;

    newtio.c_cflag = uart->baud | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_lflag = 0;
    newtio.c_oflag = 0;

    tcflush(uart->fd, TCIFLUSH);
    tcsetattr(uart->fd, TCSANOW, &newtio);

    if (sem_init_uart(&uart->sem) < 0) {
        perror("sem_init_uart() fail");
        return -1;
    }
    return 0;
}

int read_uart(uart_t *uart, char *data, unsigned int len)
{
    int ret;
    sem_wait_uart(&uart->sem);
    ret = read(uart->fd, data, len);
    sem_post_uart(&uart->sem);

    return ret;
}

int write_uart(uart_t *uart, char *data, unsigned int len)
{
    int ret;

    sem_wait_uart(&uart->sem);
    ret = write(uart->fd, data, len);
    sem_post_uart(&uart->sem);

    return ret;
}

void close_uart(uart_t *uart)
{
    sem_wait_uart(&uart->sem);
    close(uart->fd);
    uart->fd = -1;
    sem_post_uart(&uart->sem);

    sem_destroy_uart(&uart->sem);
}

int set_uart_speed(uart_t *uart)
{
    struct termios tio;
    int ret;

    sem_wait_uart(&uart->sem);

    ret = tcgetattr(uart->fd, &tio);
    if (ret < 0) {
        perror("tcgetattr() fail:");
        return -1;
    }

    tio.c_cflag = uart->baud | CS8 | CLOCAL | CREAD;

    tcflush(uart->fd, TCIFLUSH);

    ret = tcsetattr(uart->fd, TCSANOW, &tio);
    if (ret < 0) {
        perror("tcsetattr() fail:");
        return -1;
    }

    sem_post_uart(&uart->sem);

    return 0;
}