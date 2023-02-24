#ifndef _MESSAGING_H_
#define _MESSAGING_H_

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "queue.h"

#define UDPBUFSIZ 1452 
// 1500 - 40 - 8
//  mtu   ip  udp
  
#define TCPBUFSIZ 4096 
// no need to calculate 
// tcp good
// tcp nice

#define SEL_CLIENT 0
#define SEL_SERVER 1


struct client_list{
	struct sockaddr_in* client;
	struct sockaddr_in* server;
};

struct thread_arg{
	int udpsock;
	int tcpsock;
	struct queue* queue_list;
	struct client_list* client_list;
	int list_len;
	struct sockaddr_in* serveraddr;
};

int s_sendto(int sock, char* buff, int len, int flags, struct sockaddr_in* addr, int addrlen);
int s_send(int sock, char* buff, int len, int flags);

int search_list(struct client_list*, int, struct sockaddr_in*, int);

#endif
