#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "error.h" // done
#include "signals.h" // done
#include "queue.h"
#include "messaging.h" // done
#include "parse_input.h" // done
#include "front.h"
#include "conv.h"
#include "back.h"

int initialize_tun_sock(int listenport){
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0){
		eprint("Cant create udp socket\n");
		return -1;
	}
	const int one = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))){
		eprint("setsockopt error\n");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listenport);
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))){
		eprintf("Cant bind socket on %d\n", listenport);
		return -1;
	}
	return sock;
}

int initialize_xserver_conn(struct sockaddr_in* addr, size_t addrsz, int dir){
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0){
		eprint("Cant create tcp sock\n");
		return -1;
	}
	const int one = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))){
		eprint("setsockopt error");
		return -1;
	}
	if(dir == DIR_SERVER){
		addr->sin_addr.s_addr = INADDR_ANY;
		if(bind(sock, (struct sockaddr*)addr, addrsz)){
			eprint("Cannot bind socket on this port\n");
			return -1;
		}
		if(listen(sock, 10)){
			eprint("listen error\n");
			return -1;
		}
		int clientsock = accept(sock, NULL, NULL);
		if(clientsock < 0){
			eprint("accept error");
			return -1;
		}
		return clientsock;
	}else if(dir == DIR_CLIENT){
		if(connect(sock, (struct sockaddr*)addr, addrsz)){
			eprint("connect error\n");
			return -1;
		}
		return sock;
	}else{
		eprint("Unknown error\n");
		return -2;
	}
}

int main(int argc, char* argv[]){

	int listenport;
	struct sockaddr_in xserveraddr;
	struct client_list* clientaddr_list;
	int client_list_len;
	int direction = DIR_CLIENT;

	if(parse_input(argc, argv, &listenport, &xserveraddr, &clientaddr_list, &client_list_len, &direction)){
		eprint("Input error. See help for more info.\nAborting\n");
		exit(1);
	}

	struct thread_arg targ;
	targ.client_list = clientaddr_list;
	targ.list_len = client_list_len;
	targ.serveraddr = &xserveraddr;

	targ.udpsock = initialize_tun_sock(listenport);
	if(targ.udpsock<0){
		eprint("Cannot open udp socket\nAborting...\n");
	}

	targ.tcpsock = initialize_xserver_conn(&xserveraddr, sizeof(xserveraddr), direction);
	if(targ.tcpsock < 0){
		eprint("Cannot connect to xserver process. Double check your input\nAborting...\n");
		exit(1);
	}

	targ.queue_list = (struct queue*)malloc(4 * sizeof(struct queue));

	for(int i=0;i<4;i++){
		queue_initialize(targ.queue_list + i);
	}
	
	pthread_t front_thread;
	pthread_t conv_thread;
	pthread_t back_thread;

	if(pthread_create(&front_thread, NULL, front_proc, (void*)&targ)){
		eprint("Cannot start front end thread\n");
		exit(1);
	}
	if(pthread_create(&conv_thread, NULL, conv_proc, (void*)&targ)){
		eprint("Cannot start front end thread\n");
		exit(1);
	}
	if(pthread_create(&back_thread, NULL, back_proc, (void*)&targ)){
		eprint("Cannot start front end thread\n");
		exit(1);
	}

	pthread_join(front_thread, NULL);
	pthread_join(conv_thread, NULL);
	pthread_join(back_thread, NULL);

	close(targ.udpsock);
	close(targ.tcpsock);

	for(int i=0;i<4;i++){
		queue_clean(targ.queue_list + i);
	}

	free(targ.queue_list);

	exit(0);

}
