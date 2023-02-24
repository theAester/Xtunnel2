#include "messaging.h"

int s_sendto(int sock, char* buff, int len, int flags, struct sockaddr_in* addr, int addrlen){
	int sent =0;
	while(len){
		sent = sendto(sock, buff, len, flags, (struct sockaddr*)addr, addrlen);
		if(sent < 0) return 1;
		len -= sent;
		buff += sent;
	}
	return 0;
}

int s_send(int sock, char* buff, int len, int flags){
	int sent =0;
	while(len){
		sent = send(sock, buff, len, flags);
		if(sent < 0) return 1;
		len -= sent;
		buff += sent;
	}
	return 0;
}

int search_list(struct client_list* list, int len, struct sockaddr_in* addr, int selector){
	for(int i=0;i<len;i++){
		if( ( selector == SEL_SERVER &&
			list[i].server->sin_addr.s_addr == addr->sin_addr.s_addr && list[i].server->sin_port == addr->sin_port)
			||
			( selector == SEL_CLIENT &&
			list[i].client->sin_addr.s_addr == addr->sin_addr.s_addr && list[i].client->sin_port == addr->sin_port)){
			return i;
		}
	}
	return -1;
}
