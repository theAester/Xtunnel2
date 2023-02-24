#include "back.h"


void* back_proc(void* vargs){
	struct thread_arg* targ = (struct thread_arg*)vargs;
	struct queue* outqueue = targ->queue_list + 3;
	struct queue* inqueue = targ->queue_list + 2;

	fd_set rdfs;
	fd_set wrfs;
	int retval;
	struct timeval tv;

	while(!_exitsig){
		FD_ZERO(&rdfs);
		FD_ZERO(&wrfs);
		FD_SET(targ->tcpsock, &rdfs);
		if(inqueue->len > 0) FD_SET(targ->tcpsock, &wrfs);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		retval = select(1024, &rdfs, &wrfs, NULL, &tv);
		if(retval == -1){
			eprint("select error\n");
			continue;
		}else if(retval == 0){
			continue;
		}
		struct queue_node* nn;
		int datasz;
		if(FD_ISSET(targ->tcpsock, &rdfs)){
			nn = (struct queue_node*)malloc(sizeof(struct queue_node));
			nn->buffer = (char*)malloc(TCPBUFSIZ);
			if((datasz = recv(targ->tcpsock, nn->buffer, TCPBUFSIZ, 0)) <= 0){
				if(datasz == 0){
					eprint("peer closed connection\n");
					exit(1);
				}
				eprintf("tcp recv error %d\n", datasz);
				free (nn->buffer);
				free(nn);
				goto skip;
			}
			nn->datasz = datasz;
			printf("backend recv'd: %s\n", nn->buffer);
			fflush(stdout);
			queue_push(outqueue, nn);
		}
		skip:
		if(FD_ISSET(targ->tcpsock, &wrfs)){
			nn = queue_pop(inqueue);
			printf("backend sent: %s\n", nn->buffer);
			fflush(stdout);
			if(s_send(targ->tcpsock, nn->buffer, nn->datasz, 0)){
				eprint("s_sendto error\n");
				free(nn->buffer);
				free(nn);
				continue;
			}
			free(nn->buffer);
			free(nn);
		}
	}
	return NULL;
}
