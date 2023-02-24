#include "queue.h"

int queue_initialize(struct queue* queue){
	queue->len =0;
	pthread_mutex_init(&queue->lock, NULL);
	queue->head = NULL;
	queue->tail = NULL;
	return 0;
}

int queue_clean(struct queue* queue){
	queue->len =0;
	pthread_mutex_destroy(&queue->lock);
	while(queue->head != NULL){
		struct queue_node* temp = queue->head->next;
		free(queue->head);
		queue->head = temp;
	}
	return 0;
}

int queue_push(struct queue* queue, struct queue_node* node){
	node->next = NULL;
	pthread_mutex_lock(&queue->lock);
	if(queue->len == 0){
		queue->head = node;
		queue->tail = node;
	}else{
		queue->tail->next = node;
	}
	queue->len += 1;
	pthread_mutex_unlock(&queue->lock);
	return 0;
}

struct queue_node* queue_pop(struct queue* queue){
	struct queue_node* temp;
	pthread_mutex_lock(&queue->lock);
	if(queue->len == 0){
		return NULL;
	}else if(queue->len == 1){
		temp = queue->head;
		queue->head = NULL;
		queue->tail = NULL;
	}else{
		temp = queue->head;
		queue->head = temp->next;
		temp->next = NULL;
	}
	queue->len -= 1;
	pthread_mutex_unlock(&queue->lock);
	return temp;
}

