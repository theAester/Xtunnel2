#include "front.h"

void* front_proc(void* vargs){
	struct thread_arg* targ = (struct thread_arg*)vargs;
	struct queue* outqueue = targ->queue_list;
	struct queue* inqueue = targ->queue_list + 1;

	fd_set rdfs;
	fd_set wrfs;
	int retval;
	struct timeval tv;

	struct sockaddr_in tempaddr;
	unsigned int addrsz;

	while(!_exitsig){
		FD_ZERO(&rdfs);
		FD_ZERO(&wrfs);
		FD_SET(targ->udpsock, &rdfs);
		if(inqueue->len > 0) FD_SET(targ->udpsock, &wrfs);
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
		if(FD_ISSET(targ->udpsock, &rdfs)){
			nn = (struct queue_node*)malloc(sizeof(struct queue_node));
			nn->buffer = (char*)malloc(UDPBUFSIZ);
			addrsz = sizeof(tempaddr);
			if((datasz = recvfrom(targ->udpsock, nn->buffer, UDPBUFSIZ, 0, (struct sockaddr*)&tempaddr, &addrsz)) <= 0){
				eprintf("udp recvfrom error %d\n", datasz);
				free (nn->buffer);
				free(nn);
				goto skip;
			}
			nn->datasz = datasz;
			nn->index = search_list(targ->client_list, targ->list_len, &tempaddr, SEL_CLIENT);
			if(nn->index == -1){
				eprint("packet received from unknown source, dropping packet\n");
				free(nn->buffer);
				free(nn);
				goto skip;
			}
			queue_push(outqueue, nn);
		}
		skip:
		if(FD_ISSET(targ->udpsock, &wrfs)){
			nn = queue_pop(inqueue);
			struct sockaddr_in* destaddr = targ->client_list[nn->index].client;
			if(s_sendto(targ->udpsock, nn->buffer, nn->datasz, 0, destaddr, sizeof(struct sockaddr_in))){
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

