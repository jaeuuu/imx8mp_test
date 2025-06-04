
/**
 * @file api_custom_queue.c
 * @brief API using memory alloction for Inter-Thread Communication.
 * @author Jae Woo, Oh
 * @version 0.01
 * @date 2023-11-29
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <api/debug.h>
#include <api/queue.h>

 /**
  * @brief Initialize semaphore of custom queue
  *
  * @param sem sem_t variable of pointer
  *
  * @return Successful completion, returns a value of '0'. if occurs error, returns a value of '-1'.
  */
static int sem_init_queue(sem_t *sem)
{
	return sem_init(sem, 0, 1);
}


/**
 * @brief Lock semaphore of custom queue
 *
 * @param sem sem_t variable of pointer
 */
static void sem_wait_queue(sem_t *sem)
{
	sem_wait(sem);
}


/**
 * @brief Unlock semaphore of custom queue
 *
 * @param sem sem_t variable of pointer
 */
static void sem_post_queue(sem_t *sem)
{
	sem_post(sem);
}


/**
 * @brief Destroy semaphore of custom queue
 *
 * @param sem sem_t variable of pointer
 */
static void sem_destroy_queue(sem_t *sem)
{
	sem_destroy(sem);
}


/**
 * @brief Check which Queue is full or not.
 *
 * @param cq Pointer of ThrdQueue_t variable
 *
 * @return if Queue is full, returns a value of true(1). if Queue is not full, returns a value of false(0).
 */
static bool is_full(queue_t *q)
{
	return (((q->rear + 1) % q->qsize) == (q->front));
}


/**
 * @brief Check which Queue is empty or not.
 *
 * @param cq Pointer of ThrdQueue_t variable
 *
 * @return if Queue is empty, returns a value of true(1). if Queue is not empty, returns a value of false(0).
 */
static bool is_empty(queue_t *q)
{
	return ((q->rear) == (q->front));
}


/**
 * @brief Create Queue Memory in Heap Resion.
 *
 * @param cq Pointer of ThrdQueue_t variable
 * @param qsize number of Queue
 * @param dsize size per Queue
 *
 * @return Successful completion, returns a value of '0'. if occurs malloc() fail, returns a value of '-1'.
 */
int init_queue(queue_t *q, int qsize, int dsize)
{
	if (!q)
		return -1;

	if (qsize <= 0 || dsize <= 0) {
		return -1;
	}

	q->qsize = qsize;
	q->dsize = dsize;

	q->queue = malloc(q->dsize * q->qsize);
	if (!q->queue) {
		perror("malloc() fail");
		return -1;
	}
	if (sem_init_queue(&q->sem) < 0) {
		perror("sem_init() fail");
		return -1;
	}

	dlp_dbg("queue initialized! (qsize: [%d], dsize: [%d], total: [%d])", q->qsize, q->dsize, (q->qsize * q->dsize));

	return 0;
}


/**
 * @brief Receive from Queue Memory in Heap Resion.
 *
 * @param cq Pointer of ThrdQueue_t variable
 * @param buf Buffer receving Queue Data
 *
 * @return Successful completion, returns a value of '0'. if occurs null or empty error, returns a value of '-1'.
 */
int get_queue(queue_t *q, void *buf)
{
	if (!q)
		return -1;

	if (!q->queue || !buf) {
		dlp_err("queue is null");
		return -1;
	}

	if (is_empty(q)) {
		return -1;
	}

	sem_wait_queue(&q->sem);

	memcpy(buf, (q->queue + (q->front * q->dsize)), q->dsize);
	q->front = (q->front + 1) % q->qsize;

	sem_post_queue(&q->sem);

	return 0;
}


/**
 * @brief Send to Queue Memory in Heap Resion.
 *
 * @param cq Pointer of ThrdQueue_t variable
 * @param buf Buffer sending Queue Data
 *
 * @return Successful completion, returns a value of '0'. if occurs null or full error, returns a value of '-1'.
 */
int put_queue(queue_t *q, void *buf)
{
	if (!q)
		return -1;

	if (!q->queue || !buf) {
		dlp_err("queue is null");
		return -1;
	}

	if (is_full(q)) {
		dlp_warn("queue is full! (qsize: [%d], dsize: [%d])", q->qsize, q->dsize);
		return -1;
	}

	sem_wait_queue(&q->sem);

	memcpy((q->queue + (q->rear * q->dsize)), buf, q->dsize);
	q->rear = (q->rear + 1) % q->qsize;

	sem_post_queue(&q->sem);

	return 0;
}


/**
 * @brief Free Queue Memory in Heap Resion.
 *
 * @param cq Pointer of ThrdQueue_t variable
 */
int destroy_queue(queue_t *q)
{
	if (!q)
		return -1;

	if (!q->queue) {
		dlp_err("queue is null");
		return -1;
	}

	free(q->queue);
	q->qsize = 0;
	q->dsize = 0;
	q->queue = NULL;
	sem_destroy_queue(&q->sem);

	return 0;
}

int print_queue(queue_t *q, int num)
{
	if (!q)
		return -1;

	if (!q->queue) {
		dlp_err("queue is null");
		return -1;
	}

	sem_wait_queue(&q->sem);

	for (int i = 0;i < num; i++) {
		printf("===============================================%d Q=============================================\r\n", i);
		for (int j = 0; j < q->dsize; j++) {
			if ((j % 32) == 0 && j != 0) {
				printf("\r\n");
			}
			printf("%02X ", ((char *)q->queue + (i * q->dsize))[j]);
		}
		printf("\r\n===============================================================================================\r\n");
	}

	sem_post_queue(&q->sem);

	return 0;
}
