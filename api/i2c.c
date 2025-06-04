#include <api/debug.h>
#include <api/i2c.h>

/**
 * @brief I2C 장치에 접근함에 있어 동기화를 보장하며, 잠금을 생성한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
static int sem_init_i2c(sem_t *sem)
{
    return sem_init(sem, 0, 1);
}


/**
 * @brief I2C 장치에 접근함에 있어 동기화를 보장하며, 잡금을 한다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_wait_i2c(sem_t *sem)
{
    sem_wait(sem);
}


/**
 * @brief I2C 장치에 접근함에 있어 동기화를 보장하며, 잠금을 푼다.
 *
 * @param sem sem_t 타입의 포인터 변수
 */
static void sem_post_i2c(sem_t *sem)
{
    sem_post(sem);
}

static void sem_destroy_i2c(sem_t *sem)
{
    sem_destroy(sem);
}


/**
 * @brief I2C 장치 파일을 연다.
 *
 * @param i2c i2c_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int open_i2c(i2c_t *i2c)
{
    i2c->fd = open(i2c->dev, O_RDWR);
    if (i2c->fd < 0) {
        perror("open() fail");
        return -1;
    }
    if (sem_init_i2c(&i2c->sem) < 0) {
        perror("sem_init_i2c() fail");
        return -1;
    }
    return 0;
}


/**
 * @brief I2C 장치 파일을 닫는다.
 *
 * @param i2c i2c_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
void close_i2c(i2c_t *i2c)
{
    sem_wait_i2c(&i2c->sem);
    close(i2c->fd);
    sem_post_i2c(&i2c->sem);

    sem_destroy_i2c(&i2c->sem);
}


/**
 * @brief I2C 장치로부터 값을 쓴다.
 *
 * @param i2c i2c_t 타입의 포인터 변수
 * @param ctrl i2c_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int write_i2c(i2c_t *i2c, i2c_ctrl_t *ctrl)
{
    int err;
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg;
    unsigned char tmp[MAX_I2C_DATA_SIZE * 2];

    memset(&rdwr, 0x00, sizeof(struct i2c_rdwr_ioctl_data));
    memset(&msg, 0x00, sizeof(struct i2c_msg));
    memset(tmp, 0x00, sizeof(tmp));

    msg.addr = ctrl->slave_addr;

    switch (ctrl->reg_addr_len) {
    case I2C_ADDRESS_NONE:
        memcpy(tmp, ctrl->buf, ctrl->buf_len);
        msg.buf = tmp;
        msg.len = ctrl->buf_len;
        break;
    case I2C_ADDRESS_1BYTE:
        tmp[0] = (ctrl->reg_addr & 0xFF);
        memcpy(&tmp[1], ctrl->buf, ctrl->buf_len);
        msg.buf = tmp;
        msg.len = (ctrl->reg_addr_len + ctrl->buf_len);
        break;
    case I2C_ADDRESS_2BYTE:
        tmp[0] = ((ctrl->reg_addr >> 8) & 0xFF);
        tmp[1] = (ctrl->reg_addr & 0xFF);
        memcpy(&tmp[2], ctrl->buf, ctrl->buf_len);
        msg.buf = tmp;
        msg.len = (ctrl->reg_addr_len + ctrl->buf_len);
        break;
    default:
        dlp_warn("i2c address len over : %d", ctrl->reg_addr_len);
        return -1;
    }

    rdwr.msgs = &msg;
    rdwr.nmsgs = 1;

    sem_wait_i2c(&i2c->sem);
    err = ioctl(i2c->fd, I2C_RDWR, &rdwr);
    sem_post_i2c(&i2c->sem);

    return err;
}


/**
 * @brief I2C 장치로부터 값을 읽는다.
 *
 * @param i2c i2c_t 타입의 포인터 변수
 * @param ctrl i2c_ctrl_t 타입의 포인터 변수
 *
 * @return 성공이면 0, 실패면 -1
 */
int read_i2c(i2c_t *i2c, i2c_ctrl_t *ctrl)
{
    int err;
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg[2];
    unsigned char tmp[MAX_I2C_DATA_SIZE * 2];

    memset(&rdwr, 0x00, sizeof(struct i2c_rdwr_ioctl_data));
    memset(msg, 0x00, sizeof(msg));
    memset(tmp, 0x00, sizeof(tmp));

    msg[0].addr = ctrl->slave_addr;

    switch (ctrl->reg_addr_len) {
    case I2C_ADDRESS_NONE:
        msg[0].buf = tmp;
        msg[0].len = ctrl->reg_addr_len;
        break;
    case I2C_ADDRESS_1BYTE:
        tmp[0] = (ctrl->reg_addr & 0xFF);
        msg[0].buf = tmp;
        msg[0].len = ctrl->reg_addr_len;
        break;
    case I2C_ADDRESS_2BYTE:
        tmp[0] = ((ctrl->reg_addr >> 8) & 0xFF);
        tmp[1] = (ctrl->reg_addr & 0xFF);
        msg[0].buf = tmp;
        msg[0].len = ctrl->reg_addr_len;
        break;
    default:
        dlp("i2c address len over : %d", ctrl->reg_addr_len);
        return -1;
    }

    // read field
    msg[1].addr = ctrl->slave_addr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = ctrl->buf;
    msg[1].len = ctrl->buf_len;

    rdwr.msgs = msg;
    rdwr.nmsgs = 2;

    sem_wait_i2c(&i2c->sem);
    err = ioctl(i2c->fd, I2C_RDWR, &rdwr);
    sem_post_i2c(&i2c->sem);

    return err;
}
