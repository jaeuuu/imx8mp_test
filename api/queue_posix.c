
/**
 * @file api_queue_posix.c
 * @brief API using POSIX QUEUE for Inter-Process Communication.
 * @author Jae Woo, Oh
 * @version 0.01
 * @date 2023-11-29
 */


#include "api_queue_posix.h"

#define __FILENAME__ "api_queue_posix.c"
#define D_API_QUEUE_POSIX
#ifdef D_API_QUEUE_POSIX
	#define dlp(fmt, args...) printf("[%s %s(), %d]"fmt, __FILENAME__, __FUNCTION__, __LINE__, ##args)
#else
	#define dlp(fmt, args...)
#endif


/**
 * @brief Create Queue File in /dev/mqueue/.
 *
 * @param path Queue File Name
 * @param qsize number of Queue
 * @param dsize size per Queue
 *
 * @return Successful completion, returns a value of '0'. if occurs argument error or mq_open() fail, returns a value of '-1'.  
 */
int init_que_posix(const char *path, int qsize, int dsize)
{
	mqd_t mfd;
	struct mq_attr attr;

	if (!path || qsize < 0 || dsize < 0) {
		dlp("argument error\r\n");
		return -1;
	}

	attr.mq_maxmsg = qsize;
	attr.mq_msgsize = dsize;

	mq_unlink(path);

	mfd = mq_open(path, O_RDWR|O_CREAT|O_EXCL, 0644, &attr);
	if (mfd < 0) {
		perror("mq_open() fail");
		return -1;
	}
	return 0;
}


/**
 * @brief Receive from Queue File in /dev/mqueue/pathname.
 *
 * @param path Queue File Name
 * @param buf Buffer receving Queue Data
 * @param size size of Buffer or Receving size
 *
 * @return Successful completion, returns a value of '0'. if occurs argument error or mq_open() fail, returns a value of '-1'.
 */
int recv_que_posix(const char *path, void *buf)
{
	mqd_t mfd;
	struct mq_attr attr;
	struct timespec timeout;

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_nsec += 100*1000;

	if (!path || !buf) {
		dlp("argument error\r\n");
		return -1;
	}

	mfd = mq_open(path, O_RDONLY);
	if (mfd < 0) {
		perror("mq_open() fail");
		return -1;
	}

	if (mq_getattr(mfd, &attr) < 0) {
		perror("mq_getattr() fail");
		mq_close(mfd);
		return -1;
	}

	if (mq_timedreceive(mfd, buf, attr.mq_msgsize, NULL, &timeout) < 0) {
		mq_close(mfd);
		return -1;
	}

	mq_close(mfd);
	return 0;
}


/**
 * @brief Send to Queue File in /dev/mqueue/pathname.
 *
 * @param path Queue File Name
 * @param buf Buffer Sending Queue Data
 * @param size size of Buffer or Sending size
 *
 * @return Successful completion, returns a value of '0', if occurs argument error or mq_open() fail, returns a value of '-1'.
 */
int send_que_posix(const char *path, void *buf)
{
	mqd_t mfd;
	struct mq_attr attr;
	struct timespec timeout;

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_nsec += 100*1000;

	if (!path || !buf) {
		dlp("argument error\r\n");
		return -1;
	}

	mfd = mq_open(path, O_WRONLY);
	if (mfd < 0) {
		perror("mq_open() fail");
		return -1;
	}

	if (mq_getattr(mfd, &attr) < 0) {
		perror("mq_getattr() fail");
		mq_close(mfd);
		return -1;
	}

	if (mq_timedsend(mfd, buf, attr.mq_msgsize, 0, &timeout) < 0) {
		mq_close(mfd);
		return -1;
	}

	mq_close(mfd);
	return 0;
}
