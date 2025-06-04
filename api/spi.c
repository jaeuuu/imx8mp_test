#include "api_spi.h"

#define __FILENAME__ "api_spi.c"
#define D_API_SPI
#ifdef  D_API_SPI
#define dlp(fmt,args...) printf("[%s %s(), %d]"fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, ##args)
#else
#define dlp(fmt,args...)
#endif


/**
 * @brief SPI 장치에 접근함에 있어 동기화를 보장하며, 잠금을 생성한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static int sem_init_spi(sem_t *sem)
{
    return sem_init(sem, 0, 1);
}


/**
 * @brief SPI 장치에 접근함에 있어 동기화를 보장하며, 잠금을 한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_wait_spi(sem_t *sem)
{
    sem_wait(sem);
}


/**
 * @brief SPI 장치에 접근함에 있어 동기화를 보장하며, 잠금을 푼다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_post_spi(sem_t *sem)
{
    sem_post(sem);
}

static void sem_destroy_spi(sem_t *sem)
{
    sem_destroy(sem);
}


/**
 * @brief SPI 장치의 모드를 설정한다.
 *
 * @param spi spi_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int set_spi_mode(spi_t *spi)
{
    int err;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_WR_MODE, &spi->mode);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치의 비트 단위 값을 설정한다.
 *
 * @param spi spi_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int set_spi_bit_per_word(spi_t *spi)
{
    int err;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &spi->bits);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치의 비트 단위 값을 읽는다.
 *
 * @param spi spi_t 타입의 포인터 변수
 * @param bits 비트 단위 값을 읽을 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int get_spi_bit_per_word(spi_t *spi, unsigned char *bits)
{
    int err;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_RD_BITS_PER_WORD, bits);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치의 속도를 설정한다.
 *
 * @param spi spi_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int set_spi_speed(spi_t *spi)
{
    int err;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi->speed);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치의 속도를 읽는다.
 *
 * @param spi spi_t 타입의 포인터 변수
 * @param speed 속도 값을 저정할 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int get_spi_speed(spi_t *spi, unsigned char *speed)
{
    int err;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, speed);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치파일을 연다.
 *
 * @param spi spi_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int open_spi(spi_t *spi)
{
    spi->fd = open(spi->dev, O_RDWR);
    if (spi->fd < 0) {
        perror("open() fail");
        return -1;
    }
    if (sem_init_spi(&spi->sem) < 0) {
        dlp_err("sem_init_spi() fail");
        return -1;
    }
    if (set_spi_mode(spi) < 0) {
        dlp_err("set_spi_mode() fail");
        return -1;
    }
    if (set_spi_bit_per_word(spi) < 0) {
        dlp_err("set_api_bit_per_word() fail");
        return -1;
    }
    if (set_spi_speed(spi) < 0) {
        dlp_err("set_spi_speed() fail");
        return -1;
    }
    return 0;
}


void close_spi(spi_t *spi)
{
    sem_wait_spi(&spi->sem);
    close(spi->fd);
    sem_post_spi(&spi->sem);

    sem_destroy_spi(&spi->sem);
}


/**
 * @brief SPI 장치에 값을 쓴다.
 *
 * @param spi spi_t 타입의 포인터 변수
 * @param ctrl spi_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int write_spi(spi_t *spi, spi_ctrl_t *ctrl)
{
    int err;
    struct spi_ioc_transfer xfer;

    memset(&xfer, 0x00, sizeof(struct spi_ioc_transfer));
    xfer.tx_buf = (unsigned long)ctrl->tx_buf;
    xfer.len = ctrl->buf_len;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &xfer);
    sem_post_spi(&spi->sem);

    return err;
}


/**
 * @brief SPI 장치로부터 값을 읽는다.
 *
 * @param spi spi_t 타입의 포인터 변수
 * @param ctrl spi_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int read_spi(spi_t *spi, spi_ctrl_t *ctrl)
{
    int err;
    struct spi_ioc_transfer xfer;

    memset(&xfer, 0x00, sizeof(struct spi_ioc_transfer));

    xfer.tx_buf = (unsigned long)ctrl->tx_buf;
    xfer.rx_buf = (unsigned long)ctrl->rx_buf;
    xfer.len = ctrl->buf_len;

    sem_wait_spi(&spi->sem);
    err = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &xfer);
    sem_post_spi(&spi->sem);

    return err;
}
