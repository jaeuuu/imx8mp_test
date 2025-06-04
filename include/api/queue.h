#ifndef QUEUE_H__
#define QUEUE_H__

#include <semaphore.h>

#define MAX_QUEUE_SIZE		10

typedef struct st_queue {
	void *queue;
	int qsize;
	int dsize;
	int front;
	int rear;
	sem_t sem;
} queue_t;

int init_queue(queue_t *q, int qsize, int dsize);
int get_queue(queue_t *q, void *buf);
int put_queue(queue_t *q, void *buf);
int destroy_queue(queue_t *q);
int print_queue(queue_t *q, int num);

#define init_queue_for_each(addr, size, qsize, dsize) \
	for (int i = 0; i < size; i++) { \
		init_queue(&addr[i], qsize, dsize); \
	}

#endif
