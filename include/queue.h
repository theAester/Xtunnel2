#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>
#include <pthread.h>

struct queue_node {
	int index;
	char* buffer;
	int datasz;
	struct queue_node* next;
};

struct queue{
	int len;
	pthread_mutex_t lock;
	struct queue_node* head;
	struct queue_node* tail;
};

int queue_initialize(struct queue* queue);
int queue_clean(struct queue* queue);

int queue_push(struct queue* queue, struct queue_node* node);
struct queue_node* queue_pop(struct queue* queue);

#endif
