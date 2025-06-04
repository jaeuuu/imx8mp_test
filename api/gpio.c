#include <api/debug.h>
#include <api/gpio.h>

#define GPIO_DIR_SET		_IOW('g',0x01,int)	// gpio pin direction setting output
#define GPIO_OUTPUT_WRITE	_IOW('g',0x02,int)	// gpio pin output high
#define GPIO_INPUT_READ		_IOW('g',0x03,int)	// gpio pin input read value (1:high / 0:low)
#define GPIO_FREE			_IOW('g',0x04,int)	// gpio pin export free

#define SET_PIN_GPIO		_IOW('g',0x05,int)
#define SET_PIN_FUNCTION	_IOW('g',0x06,int)


/**
 * @brief GPIO 장치에 접근함에 있어 동기화를 보장하며, 잠금 장치를 생성한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int sem_init_gpio(sem_t *sem)
{
    return sem_init(sem, 0, 1);
}


/**
 * @brief GPIO 장치에 접근함에 있어 동기화를 보장하며, 잠금을 한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_wait_gpio(sem_t *sem)
{
    sem_wait(sem);
}


/**
 * @brief GPIO 장치에 접근함에 있어 동기화를 보장하며, 잠금을 푼다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_post_gpio(sem_t *sem)
{
    sem_post(sem);
}

static void sem_destroy_gpio(sem_t *sem)
{
    sem_destroy(sem);
}


/**
 * @brief GPIO 장치 특정 핀의 제어권을 해제한다.
 *
 * @param gpio gpio_t 타입의 포인터 변수
 * @param ctrl gpio_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int free_gpio(gpio_t *gpio, gpio_ctrl_t *ctrl)
{
    int err;

    sem_wait_gpio(&gpio->sem);
    err = ioctl(gpio->fd, GPIO_FREE, ctrl);
    sem_post_gpio(&gpio->sem);

    if (err < 0) {
        perror("ioctl() fail!");
        dlp_err("gpio free fail! (port: [%d], pin: [%d])", ctrl->port, ctrl->pin);
    }

    return err;
}


/**
 * @brief GPIO 장치 파일을 연다.
 *
 * @param gpio gpio_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int open_gpio(gpio_t *gpio)
{
    gpio->fd = open(gpio->dev, O_RDWR);
    if (gpio->fd < 0) {
        perror("open() fail");
        return -1;
    }
    if (sem_init_gpio(&gpio->sem) < 0) {
        perror("sem_init_gpio() fail");
        return -1;
    }
    return 0;
}


/**
 * @brief GPIO 장치로부터 값을 읽는다. (감지)
 *
 * @param gpio gpio_t 타입의 포인터 변수
 * @param ctrl gpio_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 signal, 실패면 -1
 */
int read_gpio(gpio_t *gpio, gpio_ctrl_t *ctrl)
{
    int err;

    sem_wait_gpio(&gpio->sem);
    err = ioctl(gpio->fd, GPIO_INPUT_READ, ctrl);
    sem_post_gpio(&gpio->sem);

    if (err < 0) {
        perror("ioctl() fail!");
        dlp_err("gpio read fail! (port: [%d], pin: [%d])", ctrl->port, ctrl->pin);
        return err;
    }

    dlp_dbg("gpio read! (port: [%d], pin: [%d], signal: [%s])",
        ctrl->port,
        ctrl->pin,
        ctrl->data ? "HIGH" : "LOW");

    return ctrl->data;
}


/**
 * @brief GPIO 장치에 값을 쓴다. (제어)
 *
 * @param gpio gpio_t 타입의 포인터 변수
 * @param ctrl gpio_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int write_gpio(gpio_t *gpio, gpio_ctrl_t *ctrl)
{
    int err;

    switch (ctrl->data) {
    case GPIO_LOW:
    case GPIO_HIGH:
        break;
    default:
        dlp_err("unknown gpio data");
        return -1;
    }

    sem_wait_gpio(&gpio->sem);
    err = ioctl(gpio->fd, GPIO_OUTPUT_WRITE, ctrl);
    sem_post_gpio(&gpio->sem);

    if (err < 0) {
        perror("ioctl() fail!");
        dlp_err("gpio write fail! (port: [%d], pin: [%d])", ctrl->port, ctrl->pin);
    }

    dlp_dbg("gpio write! (port: [%d], pin: [%d], signal: [%s])",
        ctrl->port,
        ctrl->pin,
        ctrl->data ? "HIGH" : "LOW");

    return err;
}


void close_gpio(gpio_t *gpio)
{
    sem_wait_gpio(&gpio->sem);
    close(gpio->fd);
    sem_post_gpio(&gpio->sem);

    sem_destroy_gpio(&gpio->sem);
}


/**
 * @brief GPIO의 IN/OUT을 설정한다.
 *
 * @param gpio gpio_t 타입의 포인터 변수
 * @param ctrl gpio_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int set_gpio_direction(gpio_t *gpio, gpio_ctrl_t *ctrl)
{
    int err;

    switch (ctrl->data) {
    case GPIO_DIR_OUTPUT:
    case GPIO_DIR_INPUT:
        break;
    default:
        dlp_err("unknown gpio direction");
        return -1;
    }

    if (free_gpio(gpio, ctrl) < 0) {
        dlp_err("free_gpio() fail");
        return -1;
    }

    sem_wait_gpio(&gpio->sem);
    err = ioctl(gpio->fd, GPIO_DIR_SET, ctrl);
    sem_post_gpio(&gpio->sem);

    if (err < 0) {
        perror("ioctl() fail!");
        dlp_err("gpio direction set fail! (port: [%d], pin: [%d])", ctrl->port, ctrl->pin);
    }

    dlp_dbg("gpio set direction! (port: [%d], pin: [%d], direction: [%s])",
        ctrl->port,
        ctrl->pin,
        ctrl->data ? "INPUT" : "OUTPUT");

    return err;
}
